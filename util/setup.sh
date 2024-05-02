#!/bin/bash

# Create .venv if it doesn't exist
if [ ! -d ".venv" ]; then
    python -m venv .venv
fi

source .venv/bin/activate

pip install -r requirements.txt