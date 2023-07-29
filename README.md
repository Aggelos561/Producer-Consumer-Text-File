# Text File Consumer-Producer

This project demonstrates a consumer-producer pattern using child processes to read lines from a text file concurrently. The producer process reads lines from the specified text file and stores them in shared memory. Each consumer process reads a line number from shared memory, retrieves the corresponding line from the text file, and writes the line back to shared memory.

## How the Project Works

1. The producer process reads lines from the specified text file and stores them in shared memory. It creates multiple consumer child processes.

2. Each consumer child process requests a line number from the shared memory.

3. The producer process receives the line number request and writes the requested line number into shared memory.

4. The consumer child process reads the line number from shared memory, retrieves the corresponding line from the text file, and writes the line back to shared memory for the producer process to read.

5. The consumers and producer work concurrently, ensuring efficient utilization of system resources.

## How to Use

1. Compile the `consumer.c` and `producer.c` programs using the provided Makefile:

   ```bash
   make all
   ```

   This command will compile both the consumer and producer programs.

2. Run the producer program with the following command:

   ```bash
   make run File=<text_file_name> K=<number_of_children> N=<number_of_requests_per_child>
   ```

   - `<text_file_name>`: The name of the text file containing the lines to be read.
   - `<number_of_children>`: The number of consumer child processes to create.
   - `<number_of_requests_per_child>`: The number of line requests each consumer child process should make.

   For example:

   ```bash
   make run File=my_text_file.txt K=5 N=10
   ```

3. The consumers and producer will execute concurrently, reading and writing lines from the text file using shared memory and semaphores for synchronization.