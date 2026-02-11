import csv
import json
import os
import subprocess
import numpy as np
from pathlib import Path
import pandas as pd


basePath = "/home/nikolas/masterthesis/DiFfRG/Nikolas/QuarkMesonDiquarkFlowFolder"
pathOfExecutable = "/home/nikolas/masterthesis/DiFfRG/Nikolas/QuarkMesonDiquarkSimple/build/QuarkMesonDiquarkLPA"
pathOfTuningResults = basePath + "/TuningResults"
pathOfCSV = basePath + "/build/output_data_running_EoM.csv"
lambdaPhiValues = np.linspace(56.0, 200.0, 10)  
m2PhiValues = np.linspace(-0.5, 0.5, 10)
m2DValues = np.linspace(0.6, 1.2, 10)
lambdaDValues = np.linspace(60.0, 200.0, 10)
lambdaDPhiValues = np.linspace(0.0, 10.0, 10)      
sigmaResultArray = np.zeros((len(lambdaPhiValues), len(m2PhiValues), len(m2DValues), len(lambdaDValues), len(lambdaDPhiValues)))
mPiResultArray = np.zeros((len(lambdaPhiValues), len(m2PhiValues), len(m2DValues), len(lambdaDValues), len(lambdaDPhiValues)))
mDResultArray = np.zeros((len(lambdaPhiValues), len(m2PhiValues), len(m2DValues), len(lambdaDValues), len(lambdaDPhiValues)))
sigmaGuess = np.nan
mPiGuess = np.nan
mDGuess = np.nan
targetSigma = 0.93
targetMPi = 0.136
targetMD = 0.6
currentBestSigma = 1000000.0
currentBestMPi = 10000000.0
currentBestMD = 10000000.0
currentBestLoss = 100000000.0
numberOfRuns = (len(lambdaPhiValues) * len(m2PhiValues) * len(m2DValues) * len(lambdaDValues) * len(lambdaDPhiValues))
finalArrayToSaveResults = np.zeros((numberOfRuns, 8))  # Columns for parameters and results
ogJsonPath = basePath + "/parameter.json"
with open(ogJsonPath, "r") as paramFile:
    parameters = json.load(paramFile)

for lambdaIndex, lambdaPhi in enumerate(lambdaPhiValues):
    for m2PhiIndex, m2Phi in enumerate(m2PhiValues):
        for m2DIndex, m2D in enumerate(m2DValues):
            for lambdaDIndex, lambdaD in enumerate(lambdaDValues):
                for lambdaDPhiIndex, lambdaDPhi in enumerate(lambdaDPhiValues):
                    runNumber = (lambdaIndex * len(m2PhiValues) * len(m2DValues) * len(lambdaDValues) * len(lambdaDPhiValues) +
                                 m2PhiIndex * len(m2DValues) * len(lambdaDValues) * len(lambdaDPhiValues) +
                                 m2DIndex * len(lambdaDValues) * len(lambdaDPhiValues) +
                                 lambdaDIndex * len(lambdaDPhiValues) +
                                 lambdaDPhiIndex + 1)
                    finalArrayToSaveResults[runNumber - 1, 0] = lambdaPhi
                    finalArrayToSaveResults[runNumber - 1, 1] = m2Phi
                    finalArrayToSaveResults[runNumber - 1, 2] = m2D
                    finalArrayToSaveResults[runNumber - 1, 3] = lambdaD
                    finalArrayToSaveResults[runNumber - 1, 4] = lambdaDPhi
                    # Update the parameters
                    parameters["lambdaPhi"] = lambdaPhi
                    parameters["m2Phi"] = m2Phi
                    parameters["m2D"] = m2D
                    parameters["lambdaD"] = lambdaD
                    parameters["lambdaDPhi"] = lambdaDPhi

                    print(f"Running for lambdaPhi={lambdaPhi}, m2Phi={m2Phi}, m2D={m2D}, lambdaD={lambdaD}, lambdaDPhi={lambdaDPhi}")
                    print(f"Run number: {runNumber} / {numberOfRuns}")

                    newJsonPath = Path(pathOfTuningResults) / f"lambdaPhi_{lambdaPhi}_m2Phi_{m2Phi}_m2D_{m2D}_lambdaD_{lambdaD}_lambdaDPhi_{lambdaDPhi}.json"

                    os.makedirs(os.path.dirname(newJsonPath), exist_ok=True)

                    # Save the updated parameters back to the file
                    with open(newJsonPath, "w") as paramFile:
                        json.dump(parameters, paramFile, indent=4)

                    proc = subprocess.run([pathOfExecutable, "-p", str(newJsonPath)], capture_output=True, text=True)
                    
                    if proc.returncode != 0:
                        print(f"Error running executable for lambdaPhi={lambdaPhi}, m2Phi={m2Phi}, m2D={m2D}, lambdaD={lambdaD}, lambdaDPhi={lambdaDPhi}")
                        print(proc.stderr)
                        continue

                    # Process the output CSV file
                    with open(pathOfCSV, "r") as csvFile:
                        csvReader = csv.reader(csvFile)
                        rows = list(csvReader)
                        irResults = rows[-1]  # Last row
                        sigmaGuess = float(irResults[2])  # Assuming sigma is in the third column
                        mPiGuess = float(irResults[4])  # Assuming mPi is in the fifth column
                        mDGuess = float(irResults[8])  # Assuming mD is in the ninth column

                        sigmaResultArray[lambdaIndex, m2PhiIndex, m2DIndex, lambdaDIndex, lambdaDPhiIndex] = sigmaGuess
                        mPiResultArray[lambdaIndex, m2PhiIndex, m2DIndex, lambdaDIndex, lambdaDPhiIndex] = mPiGuess
                        mDResultArray[lambdaIndex, m2PhiIndex, m2DIndex, lambdaDIndex, lambdaDPhiIndex] = mDGuess
                        finalArrayToSaveResults[runNumber - 1, 5] = sigmaGuess
                        finalArrayToSaveResults[runNumber - 1, 6] = mPiGuess
                        finalArrayToSaveResults[runNumber - 1, 7] = mDGuess
                    print(f"Obtained sigma: {sigmaGuess}, mPi: {mPiGuess}, mD: {mDGuess}")
                    loss = np.sqrt((sigmaGuess - targetSigma)**2 + (mPiGuess - targetMPi)**2 + (mDGuess - targetMD)**2)
                    if loss < currentBestLoss:
                        currentBestLoss = loss
                        print(f"New best parameters found: lambdaPhi={lambdaPhi}, m2Phi={m2Phi}, m2D={m2D}, lambdaD={lambdaD}, lambdaDPhi={lambdaDPhi} with sigma={sigmaGuess}, mPi={mPiGuess}, mD={mDGuess}")

                    newJsonPath.unlink()  # Delete the temporary JSON file

# Compute least squares difference

pd.DataFrame(finalArrayToSaveResults, columns=["numberOfRun", "lambdaPhi", "m2Phi", "m2D", "lambdaD", "lambdaDPhi", "sigma", "mPi", "mD"]).to_csv(basePath + "/tuning_results.csv", index=False)
leastSquaresArray = (sigmaResultArray - targetSigma)**2 + (mPiResultArray - targetMPi)**2 + (mDResultArray - targetMD)**2
minIndices = np.unravel_index(np.argmin(leastSquaresArray), leastSquaresArray.shape)
optimalLambdaPhi = lambdaPhiValues[minIndices[0]]
optimalM2Phi = m2PhiValues[minIndices[1]]
optimalM2D = m2DValues[minIndices[2]]
optimalLambdaD = lambdaDValues[minIndices[3]]
optimalLambdaDPhi = lambdaDPhiValues[minIndices[4]]
print(f"Optimal lambdaPhi: {optimalLambdaPhi}, Optimal m2Phi: {optimalM2Phi}, Optimal m2D: {optimalM2D}, Optimal lambdaD: {optimalLambdaD}, Optimal lambdaDPhi: {optimalLambdaDPhi}")
