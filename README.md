# EMovie — HTTP Server and Client (C++ / PostgreSQL)

## Overview

**EMovie** is a project consisting of an HTTP server (**EMServer**) and a console client (**EMClient**) for managing and streaming movies.  
The server is implemented in **C++** using **cpp-httplib**, **nlohmann::json**, and **PostgreSQL**.  
The client interacts with the server via HTTP, providing user authentication, session management, and content access.  

This project demonstrates backend development, database integration, session handling, and streaming.

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

---

## Usage

1. Create a PostgreSQL database:  
```bash
createdb -U <username> EMovieDB
```
2. Load database dump:
```bash
psql -U <username> -d EMovieDB -f emovie.sql
```
3. Run EMServer with database credentials
