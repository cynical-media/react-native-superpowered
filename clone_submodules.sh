#!/bin/bash
# This file is used for development only.
# Clones the subdirectories needed when developing this submodule when isolated from the real rn installation.
git clone --branch master https://github.com/nlohmann/json && git clone https://github.com/superpoweredSDK/Low-Latency-Android-iOS-Linux-Windows-tvOS-macOS-Interactive-Audio-Platform Superpowered && cd Superpowered && git checkout b37387fce81a5d1b4cda8b505d86ff74c52aa98e && git filter-branch --prune-empty --subdirectory-filter Superpowered HEAD
