import csv
import json
import os
import subprocess
import time
import numpy as np
import pandas as pd
from pathlib import Path
import optuna

basePath = "/home/nikolas/masterthesis/DiFfRG/Nikolas/QuarkMesonDiquarkComplexPotential"
pathOfExecutable = "/home/nikolas/masterthesis/DiFfRG/Nikolas/QuarkMesonDiquarkComplexPotential/build/QuarkMesonDiquarkLPA"
pathOfTuningResults = basePath + "/TuningResults"
pathOfCSV = basePath + "/build/output_data_running_EoM.csv"
ogJsonPath = basePath + "/parameter.json"
ARTIFACT_DIR = Path(basePath) / "optuna_artifacts"
ARTIFACT_DIR.mkdir(exist_ok=True)
targetSigma = 0.093
targetMPi = 0.136
targetMd = 0.600
tolSigma = 0.00465
tolMPi   = 0.00680
tolMd    = 0.0300

def black_box_function(lambdaPhi, m2Phi, m2D):
    print(f"Running for lambdaPhi={lambdaPhi}, m2Phi={m2Phi}, m2D={m2D}")
    with open(ogJsonPath, "r") as oGparamFile:
        parameters = json.load(oGparamFile)
        # Update the parameters
        parameters["physical"]["lambdaPhi"] = lambdaPhi
        parameters["physical"]["m2Phi"] = m2Phi
        parameters["physical"]["m2D"] = m2D
        #parameters["physical"]["lambdaD"] = lambdaD
        #parameters["physical"]["lambdaDPhi"] = lambdaDPhi        
    newJsonPath = Path(pathOfTuningResults) / f"lambdaPhi_{lambdaPhi}_m2Phi_{m2Phi}_m2D_{m2D}.json"
    os.makedirs(os.path.dirname(newJsonPath), exist_ok=True)
    with open(newJsonPath, "w") as paramFile:
        json.dump(parameters, paramFile, indent=4)
    try:
        proc = subprocess.run([pathOfExecutable, "-p", str(newJsonPath)], cwd=Path(pathOfExecutable).parent, capture_output=True, text=True, timeout=15*60)
    except subprocess.TimeoutExpired:
        with open(pathOfCSV, "r") as csvFile:
            csvReader = csv.reader(csvFile)
            rows = list(csvReader)
            irResults = rows[-1]  # Last row
            print(f"IR Results: {irResults}")
            sigmaGuess = float(irResults[2])  # Assuming sigma is in the third column
            mPiGuess = float(irResults[5])  # Assuming mPi is in the fifth column
            mDGuess = float(irResults[8])
            print(f"Obtained sigma: {sigmaGuess}, mPi: {mPiGuess}, mD: {mDGuess}")
        return sigmaGuess, mPiGuess, mDGuess
    if proc.returncode != 0:
        print(f"Error running executable for lambdaPhi={lambdaPhi}, m2Phi={m2Phi}, m2D={m2D}")
        print(proc.stderr)
        return np.inf, np.inf, np.inf
    with open(pathOfCSV, "r") as csvFile:
        csvReader = csv.reader(csvFile)
        rows = list(csvReader)
        irResults = rows[-1]  # Last row
        print(f"IR Results: {irResults}")
        sigmaGuess = float(irResults[2])  # Assuming sigma is in the third column
        mPiGuess = float(irResults[5])  # Assuming mPi is in the fifth column
        mDGuess = float(irResults[8])
        print(f"Obtained sigma: {sigmaGuess}, mPi: {mPiGuess}, mD: {mDGuess}")
    return sigmaGuess, mPiGuess, mDGuess

def objective(trial):
    lambdaPhi = trial.suggest_float("lambdaPhi", 20.0, 200.0)
    m2Phi = trial.suggest_float("m2Phi", -0.9, 0.9)
    m2D = trial.suggest_float("m2D", 0.5, 5.0)
    t0 = time.time()
    result = black_box_function(lambdaPhi, m2Phi, m2D)
    runtime = time.time() - t0
    trial.set_user_attr("sigma", float(result[0]))
    trial.set_user_attr("mPi", float(result[1]))
    trial.set_user_attr("mD", float(result[2]))
    trial.set_user_attr("runtime", runtime)
    trial.set_user_attr("exit_code", 0 if result[0] != np.inf and result[1] != np.inf else 1)
    trial_dir = ARTIFACT_DIR / f"trial_{trial.number}"
    trial_dir.mkdir(exist_ok=True)
    with open(trial_dir / "result.json", "w") as f:
        json.dump({"sigma": result[0], "mPi": result[1], "mD": result[2]}, f)
    if result[0] == np.inf or result[1] == np.inf or result[2] == np.inf:
        return 100000
    sigmaError = ((result[0] - targetSigma)/tolSigma) ** 2
    mPiError = ((result[1] - targetMPi)/tolMPi) ** 2
    mDError = ((result[2] - targetMd)/tolMd) ** 2
    totalError = sigmaError + mPiError + mDError
    print(f"Total error for lambdaPhi={lambdaPhi}, m2Phi={m2Phi}, mD={m2D}: {totalError}")
    return totalError

sampler = optuna.samplers.TPESampler(seed=42, n_startup_trials=30)
study = optuna.create_study(direction="minimize", sampler=sampler, study_name="QuarkMesonDiquarkComplexPotential", storage=f"sqlite:///{ARTIFACT_DIR / 'optuna_studyWoMatsubaraComplexPotential.db'}", load_if_exists=True)
study.optimize(objective, n_trials=1000)
df = study.trials_dataframe(attrs=("number", "value", "params", "user_attrs", "state", "duration"))
df.to_csv(ARTIFACT_DIR / "study_results.csv", index=False)
print("Best parameters found:")
print(study.best_params)
print("With value:")
print(study.best_value)
