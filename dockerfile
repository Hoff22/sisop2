# Dockerfile

# Use a minimal C++-friendly base
FROM gcc:13

# Set the working directory
WORKDIR /app

# Copy files to have a version in the image,
# but we will override them with volumes for development.
COPY src src
COPY include include
COPY makefile .