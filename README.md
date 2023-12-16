# RaceSync
 "RaceSync" is a streamlined software for auto race simulations, enabling real-time monitoring of critical data like fuel levels, tire condition, and lap times. It utilizes shared memory and FIFO queues for efficient data communication, essential for dynamic race strategy and management.
This code is a car racing simulation in C, employing advanced system programming concepts, including threads, shared memory, FIFOs (First In, First Out), and semaphores. Here's a detailed overview of its functionality and components:

# Structure and Functioning
Definitions and Library Inclusions: 
* The code starts with constant definitions and includes various libraries necessary for process manipulation, threads, shared memory, semaphores, and FIFOs.

Data Structures:
* StatusCarro: Stores information about the state of each car in the race (ID, lap, sector, time, fuel, tires, tire type).  
* OrganizacaoCorrida: Contains information about the teams of the cars (Car ID, team name, pilot name, team ID, available tires).

Shared Memory Functions:
* criaMemoriaPartilhada: Creates and maps a segment of shared memory.  
* apagaMemoriaPartilhada: Unmaps and removes the shared memory segment.  
* Semaphore and FIFO Functions: Several functions are used to handle semaphores and FIFOs, including creation, opening, reading, and writing in FIFOs, and finding semaphores based on team IDs.

Car Threads and Master Thread:
* lerTelemetria: Processes the telemetry of each car, reading information from a FIFO, and determines if the car needs to pit.  
* threadMaster: Controls the overall state of the race and responds to specific events such as the race start and car crashes.

Auxiliary Functions:
* buscaPneusDisponiveis, buscaNomePiloto, buscaEquipa: Functions to fetch specific car information from shared memory.  
* compara: Comparison function for sorting cars.  
* imprimeResultados: Prints the race results.  
* escreveTC: Writes a message to a specific FIFO.  
* tiraUmCentilitro: Decreases the amount of fuel in the shared reservoir.

Race Finishing and Control:
* terminaCorridaAbruptamente: Abruptly ends the race.  
* bandeiraAmarelaLevantada and bandeiraAmarelaRecolhida: Functions for race event control.  

Main Function (main):
* Initializes semaphores and threads, sets signals and alarms, and executes the main race control.  
* Creates and manages threads for each car and the master thread.  
* Prints the results and cleans up resources at the end.  

# Utilized Concepts
* Threads: To simulate the race of each car and the master thread controlling the overall state.  
* Shared Memory: For sharing information between processes, such as the state of cars and teams.  
* FIFOs (Named Pipes): For inter-process communication, enabling telemetry data reading and writing and race control.  
* Semaphores: For synchronization between threads and processes, ensuring controlled access to shared resources.  
* Signals and Alarms: To handle external events and time conditions in the simulation.  
* File Handling and Error Management: Uses functions for creating, opening, reading, writing, and closing FIFOs, along with error handling.  
