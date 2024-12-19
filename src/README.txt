# BitTorrent Simulation in OpenMPI

## Project Overview

This project implements a BitTorrent simulation using the OpenMPI framework, following the BitTorrent protocol:

- **Peer-to-Peer File Sharing**: Efficient file distribution among multiple peers.
- **Tracker Management**: A centralized tracker to monitor files and manage peers/seeders.

The project follows [Google's C++ Style Guide](https://google.github.io/styleguide/cppguide.html).

---

## Project Structure

The project is organized into the following components:

| **File**                  | **Description**                                                         |
|---------------------------|-------------------------------------------------------------------------|
| `main.cpp`                | Entry point of the program.                                            |
| `tracker.cpp`             | Implementation of the tracker class, responsible for file and peer management. |
| `tracker.h`               | Header file for the tracker class.                                     |
| `client.cpp`              | Implementation of the client class, simulating a BitTorrent peer.      |
| `client.h`                | Header file for the client class.                                      |
| `auxillary.cpp`           | Implementation of auxiliary utility functions, used by both tracker and peers                        |
| `auxillary.h`             | Header file for auxiliary functions.                                   |
| `thread_functions.cpp`    | Implementation of thread functions utilized by the client class.       |
| `thread_functions.h`      | Header file for thread-related functions.                              |
| `type.h`                  | Header file defining OpenMPI-specific data types / structures used throughout the project. |
| `type.cpp`               | Functions     to  create the OpenMPI types           |
| `Makefile`                | Makefile for compiling the project.                                    |

---

#### **Communication Model**
1. **MPI Communication**:
   - All hosts communicate using MPI messages with pre-defined tags, as defined in `type.h`.

2. **Message Tags**:
   - Each type of message is identified using unique MPI tags:
     - **`TAG_PROBING`**: Requesting swarm information from the tracker.
     - **`TAG_INQUIRY`**: Client-to-client fragment request.
     - **`TAG_KILL_ALL`**: Tracker instructs upload threads to terminate.
     - Other tags as defined in `type.h`.

---

#### **Client**
1. **Threading**:
   - Each client runs two threads:
     - **Download Thread**:
       - Handles downloading file fragments.
       - Communicates with the tracker and other clients.
     - **Upload Thread**:
       - Handles serving fragment requests from other clients.
       - Waits for messages from other clients or the tracker.
       - Notifies the tracker once an upload is completed to ensure even distribution of upload jobs.

2. **Downloading**:
   - A client requests a swarm for the file
   - The tracker responds with:
     - **Peers/Seeders**: A list of clients holding the file fragments.
     - **File Fragment Hashes**: Used for data integrity verification.
     - **Busyness Levels**: Number of current uploads per client (used for choosing the least busy client).

   - The client:
     - Chooses the least busy client holding the required fragment.
     - Downloads up to `DOWNLOAD_LIMIT` fragments per file before a new interogation.
     - Downloads `DOWNLOAD_LIMIT` fragments alternately across files to ensure no file blocks the download of another.

3. **Tracker Notifications**:
   - On receiving a fragment, the client announces the tracker to update the peers list for the file.
   - Upon completing a file:
     - The client notifies the tracker that it is now a seeder for the file.
   - Upon completing all files:
     - The client notifies the tracker that its download thread has terminated.

4. **Uploading**:
    - The upload thread handles two types of messages:
     - **`TAG_INQUIRY`**:
       - From another client requesting a file fragment.
       - Searches for the requested fragment in the client's files (including partial files if the client is a peer).
       - Sends an acknowledgment (ACK) if the fragment is available.
     - **`TAG_KILL_ALL`**:
       - From the tracker signaling the thread to terminate.

---

#### **Tracker**
1. **Probing for Client Messages**:
   - The tracker continuously probes for messages from clients:
     - **Fragment Requests**:
       - Responds with the swarm details for the requested file
     - **File/Fragment Completion Notifications**:
       - Updates the seeder/peer list for the file.
     - **Client Completion Notification**:
       - Tracks the status of download threads for all clients.

2. **Tracker Termination**:
   - Once all clients have completed downloading all their files, the tracke sends a **`TAG_KILL_ALL`** message to terminate the upload threads of all clients.

### **Further Details**
1. **Initialization**:
   - Tracker starts and waits for the initialization messages from all clients.
   - It broadcast an `ACK` and all clients can start their upload/download threads.
   - It starts listening for further requests.

2. **Synchronization**:
   - Both the upload and download thread  have access to the partial files of the client, therefore it must be ensured that the uploader will   `NOT` read/send a fragment, while the downloader is writing.
   - A `mutex` variable was used:
    - Once the downloader receives a new fragments it locks the access to the partial_files, adds the new fragment and unlock the mutex
    - An uploader will try to access the partial files through the mutex in order to read/send a fragment correctly

3. **Upload Distribution**
    - The tracker maintains a list of the number of fragments uploaded by each client.
    - The clients receive this list and choose the client will the least number of uploads.
    - The system ensures all peers/seeders contribute evenly to the total number of uploads.

