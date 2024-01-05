# Multi-User Messaging Application

## Overview

This project involves the development of a multi-user messaging application using the C programming language. Fundamental information technologies such as socket programming, threads, and mutex are employed to create a robust communication platform. The application operates on a server-client architecture, facilitating real-time and concurrent interactions among users.

## Primary Objectives

### Multi-User Communication
The application enables multiple users to communicate through a central server. Each user can connect to the server as an independent client, participate in parallel conversations, and engage in instant message exchanges.

### Socket Programming
The interaction between the server and clients is built upon the principles of socket programming. This ensures reliable data exchange and facilitates real-time communication.

### Utilization of Threads
Threads are extensively employed in the project to enhance multitasking capabilities. This allows multiple users to interact simultaneously, exchange messages, and accelerates the processing of operations.

### Mutex and Synchronization
The use of mutex is incorporated to ensure data integrity and synchronization among threads. This prevents data conflicts when multiple processes occur simultaneously.

# Getting Started

## System Compatibility

The software has been tested on Ubuntu 22.04.

## Database Configuration

The software utilizes the following folders and files as its database:
- `messages` folder
- `contacts` folder
- `users.csv` file

Make sure these components are present in your system for proper functionality.

## Execution Instructions

### Linux System

1. Open a terminal and run the following command to start the server:
   `bash server.txt`
2. Open another terminal and run the following command to start the client:
   `bash client.txt`

(You can run the same command in a different terminal for another client.)
Note: The software is configured with MAX_CLIENTS = 10, so ensure a maximum of 5 client connections to accommodate memory organization.

## Report
You can find detailed technical information about the project in the `report/report.pdf` file.

