#!/usr/bin/env bash
set -ev

pip install cibuildwheel==1.5.5

if [[ $RUNNER_OS == "Windows" ]]; then
  choco install -y swig --version=3.0.12
  pip install wheel numpy==1.18.5
elif [[ $RUNNER_OS == "macOS" ]]; then
  brew install swig
  pip install wheel numpy==1.18.5
fi
