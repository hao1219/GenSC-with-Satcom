#!/bin/bash

ENV_NAME="sat-gensc-env"
PYTHON_SCRIPT="GenSC_finall_all.py"  

if conda info --envs | grep -q "$ENV_NAME"; then
    echo "Conda environment $ENV_NAME already exists."
else
    echo "Creating conda environment $ENV_NAME..."
    conda env create -f environment.yml
fi

echo "Activating conda environment..."
source activate $ENV_NAME

echo "Running Python script..."
python $PYTHON_SCRIPT

echo "Deactivating conda environment..."
conda deactivate
