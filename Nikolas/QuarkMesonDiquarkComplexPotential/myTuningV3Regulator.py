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

def black_box_function(sAmp, sWidth):
    print(f"Running for sAmp={sAmp}, sWidth={sWidth}")
    with open(ogJsonPath, "r") as oGparamFile:
        parameters = json.load(oGparamFile)
        # Update the parameters
        parameters["physical"]["sAmp"] = sAmp
        parameters["physical"]["sWidth"] = sWidth     
    newJsonPath = Path(pathOfTuningResults) / f"sAmp_{sAmp}_sWidth_{sWidth}.json"
    os.makedirs(os.path.dirname(newJsonPath), exist_ok=True)
    with open(newJsonPath, "w") as paramFile:
        json.dump(parameters, paramFile, indent=4)
    try:
        proc = subprocess.run([pathOfExecutable, "-p", str(newJsonPath)], cwd=Path(pathOfExecutable).parent, capture_output=True, text=True, timeout=10*60)
    except subprocess.TimeoutExpired:
        return np.inf, np.inf
    if proc.returncode != 0:
        print(f"Error running executable for sAmp={sAmp}, sWidth={sWidth}")
        print(proc.stderr)
        return np.inf, np.inf, np.inf
    with open(pathOfCSV, "r") as csvFile:
        csvReader = csv.reader(csvFile)
        rows = list(csvReader)
        irResults = rows[-1]  # Last row
        print(f"IR Results: {irResults}")
        sigmaGuess = float(irResults[2])  # Assuming sigma is in the third column
        mPiGuess = float(irResults[5])  # Assuming mPi is in the fifth column
        #mDGuess = float(irResults[8])
        print(f"Obtained sigma: {sigmaGuess}, mPi: {mPiGuess}")
    return sigmaGuess, mPiGuess

def objective(trial):
    sAmp = trial.suggest_float("sAmp", 0.001, 1.0)
    sWidth = trial.suggest_float("sWidth", 0.001, 1.0)
    t0 = time.time()
    result = black_box_function(sAmp, sWidth)
    runtime = time.time() - t0
    trial.set_user_attr("sigma", float(result[0]))
    trial.set_user_attr("mPi", float(result[1]))
    trial.set_user_attr("runtime", runtime)
    trial.set_user_attr("exit_code", 0 if result[0] != np.inf and result[1] != np.inf else 1)
    trial_dir = ARTIFACT_DIR / f"trial_{trial.number}"
    trial_dir.mkdir(exist_ok=True)
    with open(trial_dir / "result.json", "w") as f:
        json.dump({"sigma": result[0], "mPi": result[1]}, f)
    if result[0] == np.inf or result[1] == np.inf:
        return 100000
    sigmaError = ((result[0] - targetSigma)/tolSigma) ** 2
    mPiError = ((result[1] - targetMPi)/tolMPi) ** 2
    totalError = sigmaError + mPiError
    print(f"Total error for sAmp={sAmp}, sWidth={sWidth}: {totalError}")
    return totalError

sampler = optuna.samplers.TPESampler(seed=42, n_startup_trials=30)
study = optuna.create_study(direction="minimize", sampler=sampler, study_name="QuarkMesonDiquarkComplexPotentialRegulator", storage=f"sqlite:///{ARTIFACT_DIR / 'optuna_studyWoMatsubaraComplexPotentialRegulator.db'}", load_if_exists=True)
study.optimize(objective, n_trials=1000)
df = study.trials_dataframe(attrs=("number", "value", "params", "user_attrs", "state", "duration"))
df.to_csv(ARTIFACT_DIR / "study_results.csv", index=False)
print("Best parameters found:")
print(study.best_params)
print("With value:")
print(study.best_value)
