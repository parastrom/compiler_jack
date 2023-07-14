cd build

# Path: build.sh

# Step 1: Build the project.
cmake -GNinja ..

# Step 2: Compile the project.
ninja

# Step 3: Run the project.
./compiler

# Step 4: Clean the project.
ninja clean

# Step 5: Return to the root directory.
cd ..