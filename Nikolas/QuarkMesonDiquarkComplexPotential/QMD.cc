#include <DiFfRG/common/configuration_helper.hh>
#include <DiFfRG/common/utils.hh>
#include <DiFfRG/discretization/discretization.hh>
#include <DiFfRG/timestepping/timestepping.hh>
#include <spdlog/spdlog.h>

#include "model.hh"

using namespace dealii;
using namespace DiFfRG;

using Model = QuarkMesonDiquarkLPA;
constexpr uint dim = Model::dim;
using Discretization = CG::Discretization<Model::Components, double, RectangularMesh<dim>>;
using VectorType = typename Discretization::VectorType;
using SparseMatrixType = typename Discretization::SparseMatrixType;
using Assembler = CG::Assembler<Discretization, Model>;
using TimeStepper = TimeStepperSUNDIALS_IDA_BoostABM<VectorType, SparseMatrixType, dim, GMRES>;

// Adapter for SUNDIALS IDA: provides exactly two blocks (y, ydot) via AbstractFlowingVariables.
template <typename NumberType>
class IDAFlowingVariables final : public DiFfRG::AbstractFlowingVariables<NumberType>
{
public:
  using BlockVector = dealii::BlockVector<NumberType>;
  using Vector      = dealii::Vector<NumberType>;

  explicit IDAFlowingVariables(const std::vector<unsigned int> &block_sizes)
  {
    data_.reinit(block_sizes); // must be size 2 for IDA
  }

  BlockVector &data() override { return data_; }
  const BlockVector &data() const override { return data_; }

  // Interpret "spatial_data" as y and "variable_data" as ydot for IDA.
  Vector &spatial_data() override { return data_.block(0); }
  const Vector &spatial_data() const override { return data_.block(0); }

  Vector &variable_data() override { return data_.block(1); }
  const Vector &variable_data() const override { return data_.block(1); }

private:
  BlockVector data_;
};


int main(int argc, char *argv[])
{
    ConfigurationHelper config(argc, argv);
    const auto json = config.get_json();

    Model model(json);
    RectangularMesh<dim> mesh(json);
    Discretization discretization(mesh, json);
    Assembler assembler(discretization, model, json);
    TimeStepper time_stepper(json, &assembler);

    //FE::FlowingVariables initial_condition(discretization);
    //initial_condition.interpolate(model);

    // 1) Build the UV state in the usual way
    FE::FlowingVariables<Discretization> uv(discretization);
    uv.interpolate(model);

    // 2) Flatten uv.data() into a single vector y, then build (y, ydot) for IDA
    const auto &uv_data = uv.data();

    // total size of the concatenated state vector
    unsigned int n_state = 0;
    for (unsigned int b = 0; b < uv_data.n_blocks(); ++b)
    n_state += uv_data.block(b).size();

    // IDA requires exactly two blocks: y and ydot (same size)
    IDAFlowingVariables<double> ida_ic({n_state, n_state});

    // Copy uv -> ida_ic.block(0)
    unsigned int offset = 0;
    for (unsigned int b = 0; b < uv_data.n_blocks(); ++b) {
    const auto &src = uv_data.block(b);
    for (unsigned int i = 0; i < src.size(); ++i)
        ida_ic.data().block(0)[offset + i] = src[i];
    offset += src.size();
    }

    // Bootstrap: ydot(t0) = 0 (may be replaced by a consistent value later)
    ida_ic.data().block(1) = 0.0;

    Timer timer(MPI_COMM_WORLD, true);
    timer.start();
    try{
        //time_stepper.run(&initial_condition, 0., json.get_double("/timestepping/final_time"));
        time_stepper.run(&ida_ic, 0., json.get_double("/timestepping/final_time"));
    } catch (std::exception &e){
        spdlog::get("log")->error("Simulation finished with exception {}", e.what());
        return -1;
    }
    timer.stop();
    double time = timer.wall_time();

    assembler.log("log");
    spdlog::get("log")->info("Simulation finished after {:.3f} s", time);
    return 0;
}