import csv
import json
import os
import subprocess
import time
import numpy as np
import pandas as pd
from pathlib import Path
import adaptive
from adaptive.runner import SequentialExecutor

basePath = "/home/spindler-old/Code/Other/DiFfRG/Nikolas/QuarkMesonDiquarkComplexPotential"
pathOfExecutable = basePath + "/build/QuarkMesonDiquarkLPA"
pathOfPhaseDiagram = basePath + "/PhaseDiagram"
pathOfCSVphysical = basePath + "/build/output_data_running_EoM.csv"
pathOfCSVchiral = basePath + "/build/output_data_chiral.csv"
ogJsonPath = basePath + "/parameter.json"
ADAPTIVE_DIR = Path(basePath) / "adaptive_artifacts"
os.makedirs(ADAPTIVE_DIR, exist_ok=True)

lossGoal = 0.001

def solvePoint(point):
    T, muq = point
    print(f"Running for T={T}, muq={muq}")
    phaseSpacePointPath = Path(pathOfPhaseDiagram) / f"T_{T}_muq_{muq}"
    pointAlreadyCalculated = (phaseSpacePointPath / f"T_{T}_muq_{muq}_results.csv").exists()
    if pointAlreadyCalculated:
        print(f"Point T={T}, muq={muq} already calculated. Skipping execution.")
        with open(phaseSpacePointPath / f"T_{T}_muq_{muq}_results.csv", "r") as csvFile:
            csvReader = csv.reader(csvFile)
            rows = list(csvReader)
            irResults = rows[-1]  # Last row
            sigmaGuess = float(irResults[2])  # Assuming sigma is in the third column
            print(f"Obtained sigma from existing results: {sigmaGuess}")
            return sigmaGuess
    os.makedirs(phaseSpacePointPath, exist_ok=True)
    with open(ogJsonPath, "r") as oGparamFile:
        parameters = json.load(oGparamFile)
        # Update the parameters
        parameters["physical"]["T"] = T
        parameters["physical"]["muq"] = muq      
    newJsonPath = phaseSpacePointPath / f"T_{T}_muq_{muq}.json"
    os.makedirs(os.path.dirname(newJsonPath), exist_ok=True)
    with open(newJsonPath, "w") as paramFile:
        json.dump(parameters, paramFile, indent=4)
    try:
        proc = subprocess.run([pathOfExecutable, "-p", str(newJsonPath)], cwd=Path(pathOfExecutable).parent, capture_output=True, text=True, timeout=5*60)
    except subprocess.TimeoutExpired:
        print(f"Execution timed out for T={T}, muq={muq}")
        with open(pathOfCSVphysical, "r") as csvFile:
            csvReader = csv.reader(csvFile)
            rows = list(csvReader)
            physicalIrResults = rows[-1]  # Last row
            print(f"IR Results: {physicalIrResults}")
            sigmaGuess = float(physicalIrResults[2])  # Assuming sigma is in the third column
            print(f"Obtained sigma: {sigmaGuess}")
        with open(pathOfCSVchiral, "r") as csvFile:
            csvReader = csv.reader(csvFile)
            rows = list(csvReader)
            chiralIrResults = rows[-1]  # Last row
            print(f"IR Results: {chiralIrResults}")
            sigmaGuessChiral = float(chiralIrResults[2])  # Assuming sigma is in the third column
            print(f"Obtained chiral sigma: {sigmaGuessChiral}")
        physicalDataframe = pd.read_csv(pathOfCSVphysical)
        chiralDataframe = pd.read_csv(pathOfCSVchiral)
        combinedDataframe = physicalDataframe.merge(chiralDataframe, on="t", suffixes=("_physical", "_chiral"))
        resultsCSVPath = phaseSpacePointPath / f"T_{T}_muq_{muq}_results.csv"
        combinedDataframe.to_csv(resultsCSVPath, index=False)
        return sigmaGuess
    if proc.returncode != 0:
        with open(pathOfCSVphysical, "r") as csvFile:
            csvReader = csv.reader(csvFile)
            rows = list(csvReader)
            physicalIrResults = rows[-1]  # Last row
            print(f"IR Results: {physicalIrResults}")
            sigmaGuess = float(physicalIrResults[2])  # Assuming sigma is in the third column
            print(f"Obtained sigma: {sigmaGuess}")
        with open(pathOfCSVchiral, "r") as csvFile:
            csvReader = csv.reader(csvFile)
            rows = list(csvReader)
            chiralIrResults = rows[-1]  # Last row
            print(f"IR Results: {chiralIrResults}")
            sigmaGuessChiral = float(chiralIrResults[2])  # Assuming sigma is in the third column
            print(f"Obtained chiral sigma: {sigmaGuessChiral}")
        physicalDataframe = pd.read_csv(pathOfCSVphysical)
        chiralDataframe = pd.read_csv(pathOfCSVchiral)
        combinedDataframe = physicalDataframe.merge(chiralDataframe, on="t", suffixes=("_physical", "_chiral"))
        resultsCSVPath = phaseSpacePointPath / f"T_{T}_muq_{muq}_results.csv"
        combinedDataframe.to_csv(resultsCSVPath, index=False)
        return sigmaGuess
    with open(pathOfCSVphysical, "r") as csvFile:
        csvReader = csv.reader(csvFile)
        rows = list(csvReader)
        physicalIrResults = rows[-1]  # Last row
        print(f"IR Results: {physicalIrResults}")
        sigmaGuess = float(physicalIrResults[2])  # Assuming sigma is in the third column
        print(f"Obtained sigma: {sigmaGuess}")
    with open(pathOfCSVchiral, "r") as csvFile:
        csvReader = csv.reader(csvFile)
        rows = list(csvReader)
        chiralIrResults = rows[-1]  # Last row
        print(f"IR Results: {chiralIrResults}")
        sigmaGuessChiral = float(chiralIrResults[2])  # Assuming sigma is in the third column
        print(f"Obtained chiral sigma: {sigmaGuessChiral}")
    physicalDataframe = pd.read_csv(pathOfCSVphysical)
    chiralDataframe = pd.read_csv(pathOfCSVchiral)
    combinedDataframe = physicalDataframe.merge(chiralDataframe, on="t", suffixes=("_physical", "_chiral"))
    resultsCSVPath = phaseSpacePointPath / f"T_{T}_muq_{muq}_results.csv"
    combinedDataframe.to_csv(resultsCSVPath, index=False)
    return sigmaGuess

learner = adaptive.Learner2D(solvePoint, bounds=[(0.001, 0.25), (0.001, 0.3)])
# learner = adaptive.Learner2D(solvePoint, bounds=[(0.25, 0.5), (0.001, 0.3)])
# learner = adaptive.Learner2D(solvePoint, bounds=[(0.001, 0.25), (0.3, 0.6)])
# learner = adaptive.Learner2D(solvePoint, bounds=[(0.25, 0.5), (0.3, 0.6)])
# learner = adaptive.Learner2D(solvePoint, bounds=[(0.001, 0.25), (0.6, 0.9)])
# learner = adaptive.Learner2D(solvePoint, bounds=[(0.25, 0.5), (0.6, 0.9)])
runner = adaptive.BlockingRunner(learner, executor=SequentialExecutor(), loss_goal=lossGoal)