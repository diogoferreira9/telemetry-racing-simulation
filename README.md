# Racing Simulation: Team Telemetry and Resource Management

## Overview
The Racing Simulation project is a comprehensive multi-threaded application developed in C, simulating a car racing scenario. It focuses on team telemetry data handling, resource management (fuel and tires), and inter-process communication using shared memory, FIFOs, and semaphores. This project demonstrates complex synchronization mechanisms and IPC techniques in a real-time system context.

## Features
- **Telemetry Data Processing**: Each car's telemetry data (e.g., fuel level, tire condition, lap times) is read from individual FIFOs and processed to make strategic decisions, such as pit stops.
- **Resource Management**: The application manages shared resources like fuel and tires, ensuring proper access control with semaphores to avoid race conditions.
- **Real-Time Race Control**: Signals and alarms are used to handle race events, such as the start of the race, yellow flags (caution periods), and race termination due to time constraints.
- **Team Synchronization**: Team-specific semaphores are used to synchronize actions like pit stops, ensuring that only one car from a team can be serviced at a time.
- **Robust Inter-Process Communication**: The application uses FIFOs for communication between the race control (master thread) and individual cars (threads), and shared memory for shared data like team organization and global fuel reserve.

## Main Components
- **Car Threads**: Each car in the race is represented by a thread that processes its telemetry data and makes decisions based on its current status and resources.
- **Race Master Thread**: A master thread controls the race's overall flow, handling start, flags, and termination signals. It also processes and reacts to critical events, such as car crashes or despistes.
- **Shared Memory**: Shared memory segments are used for storing information accessible to multiple processes, like the car's status and team organization data.
- **FIFOs**: Named pipes (FIFOs) are used for communication between the car threads and the race master, transmitting telemetry data and control messages.
- **Semaphores**: Semaphores are employed to manage access to shared resources, synchronize team-specific operations, and coordinate the race's start.

## How to Run
1. **Compile the Code**: Use a C compiler to compile the source code.
2. **Set Up**: Initialize the shared resources and start the race master thread.
3. **Start the Race**: Car threads begin processing telemetry data and responding to race events.
4. **Monitor and Control**: The race master monitors the race's progress, handles flags, and can terminate the race.
5. **Finish**: At the end of the race or upon termination, resources are cleaned up, and results are displayed.

## Conclusion
This Racing Simulation project offers a detailed glimpse into the complexities of real-time systems and the synchronization of multiple threads and processes. It showcases how various IPC mechanisms can be utilized in a high-stakes environment like car racing, where timely and synchronized operations are crucial.

---

Enjoy the simulation and may the fastest team win!