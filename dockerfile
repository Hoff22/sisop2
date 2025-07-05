# Use a minimal C++-friendly base
FROM gcc:13

WORKDIR /app
COPY src src
COPY include include
COPY makefile .

# Compile the server
RUN make debug

# Run the server on container start
CMD ["./bin/debug/server", "4040", "42"]
