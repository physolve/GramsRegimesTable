#!/bin/bash
# Generate Doxygen XML
doxygen Doxyfile

# Generate Sphinx HTML
sphinx-build -b html . _build
