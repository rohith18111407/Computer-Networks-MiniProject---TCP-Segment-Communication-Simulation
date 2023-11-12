# Computer-Networks-MiniProject---TCP-Segment-Communication-Simulation

The repository contains the necessary codes for TCP-Segment-Communication-Simulation

Overview
The goal of this project is to design and implement a simplified Transmission Control Protocol (TCP) Segment Communication Simulation. TCP is a widely used transport layer protocol responsible for reliable data transmission over a network. In this simulation, we will focus on the allocation of the segment numbers based on the number of bytes transmitted and acknowledgment numbers.

Requirements
C compiler ( e.g GCC) Operating System with Socket Support (Linux)

Getting started
Clone the repository
git clone

Navigate to the project directory
cd tcp-segment-communication-simulation

Alternatively
Download The Files Locally And Follow As Given Below

Compile the client and server programs
gcc TCP-server.c -o server
gcc TCP-client.c -o client

Run the server
./server 1234

Run the client
./client 1234

Usage
The client and server programs have interactive interfaces for inputting parameters and displaying allocated segment numbers and acknowledgments. Follow the on-screen prompts to initiate communication, request segment allocation, and simulate data transmission.

Algorithm
An algorithm that determines how segment numbers are allocated based on the number of bytes transmitted by the client. Considered factors such as sequence numbers and acknowledgment numbers in the TCP header.

User Interface
Created a simple user interface for both the client and server sides to input necessary parameters and displayed the allocated segment numbers and acknowledgment information.

Documentation
Provided comprehensive documentation that includes the design rationale, algorithms used, implementation details, and instructions for running the simulation.
