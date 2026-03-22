# EMovie — HTTP Server and Client (C++ / PostgreSQL)

## Overview

**EMovie** is a project consisting of an HTTP server (**EMServer**) and a console client (**EMClient**) for managing and streaming movies.  
The server is implemented in **C++** using **cpp-httplib**, **nlohmann::json**, and **PostgreSQL**.  
The client interacts with the server via HTTP, providing user authentication, session management, and content access.  

This project demonstrates backend development, database integration, session handling, and streaming.

---

## Install
Make sure you have the PostgreSQL development libraries (libpq-dev) installed.

1. Clone the repository and build project:
``` bash
git clone https://github.com/Elik565/EMovie.git
cd EMovie
cmake -S . -B build
cmake --build build
```
2. Create database:
``` bash
# make sure that the user exists and has permission to connect and create the database
psql -U <username> -d EMovieDB -f emovie.sql
```

---
## Usage
Before running the server, make sure that:
- A database user is created
- The user has access to the database and (optionally) a password
  
1. Run EMServer with database credentials:
``` bash
cd build
./server <username> <password>
```
2. Run EMClient:
``` bash
./client
```
Now you can select a movie to watch or update the data if you are an administrator.

To make a client an administrator, connect to the database and set the 'can_modify' field to true in the users table for the corresponding user.

### How to add a movie

Make sure ffmpeg is installed on your system

1. Download or copy the movie into the Movies directory
2. Create a folder for your movie:
``` bash
mkdir Movies/your_movie
```
3. Generate an HLS playlist with segments:
``` bash
ffmpeg -i Movies/your_movie.mp4 \
       -codec: copy \
       -start_number 0 \
       -hls_time 5 \  # how many seconds will be in segment
       -hls_list_size 0 \
       -f hls Movies/your_movie/your_movie.m3u8
```
4. Add the movie to the database using the client app or directly via SQL
5. Optionally, delete the original source file.
---

## Features

### EMServer

- User registration and authentication with token-based sessions
- Session management using JWT-like tokens
- Movie listing and HLS video streaming (HTTP Live Streaming)
- Role-based access control (user/admin)
- Movie management (adding new movies for admins)

**HTTP Endpoints:**

- `POST /reg` — register a new user  
- `POST /auth` — login and receive session token  
- `POST /logout` — end user session  
- `GET /movie_list` — get list of available movies  
- `GET /watch?title=<name>` — get HLS playlist for a movie  
- `POST /add_movie` — add a movie (admin only)  

### EMClient

- Console-based client for interacting with **EMServer**
- Handles user registration, login, and session management
- Fetches and displays movie list
- Plays selected movies via VLC Media Player
- Allows admins to add movies
- Handles session renewal if the server restarts

---

## Architecture

- **EMServer** — main server class, handles HTTP requests and sessions  
- **EMDatabase** — PostgreSQL integration module  
- **HLS module** — supports playlists and `.ts` video segments  
- **JWT-like tokens** — manage authentication and session information  
