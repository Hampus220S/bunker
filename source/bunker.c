/*
 * bunker
 *
 * Written by Hampus Fridholm
 *
 * Last updated: 2024-11-25
 */

#include <stdio.h>
#include <argp.h>
#include <stdlib.h>
#include <string.h>

#include "file.h"

static char doc[] = "bunker - a secure chat room";

static char args_doc[] = "[INFO...]";

static struct argp_option options[] =
{
  { "name", 'n', "NAME", 0, "Your nickname in the room" },
  { "room", 'r', "ROOM", 0, "New name of chat room" },
  { 0 }
};

struct args
{
  char** args;
  size_t arg_count;
  char*  name;
  char*  room;
};

struct args args =
{
  .args      = NULL,
  .arg_count = 0,
  .name      = NULL,
  .room      = NULL
};

/*
 * This is the option parsing function used by argp
 */
static error_t opt_parse(int key, char* arg, struct argp_state* state)
{
  struct args* args = state->input;

  switch(key)
  {
    case 'n':
      args->name = arg;
      break;

    case 'r':
      args->room = arg;
      break;

    case ARGP_KEY_ARG:
      args->args = realloc(args->args, sizeof(char*) * (state->arg_num + 1));

      if(!args->args) return ENOMEM;

      args->args[state->arg_num] = arg;

      args->arg_count = state->arg_num + 1;
      break;

    case ARGP_KEY_END:
      break;

    default:
      return ARGP_ERR_UNKNOWN;
  }

  return 0;
}

/*
 * RETURN (int status)
 * - 0 | Success
 * - 1 | String argument not allocated
 * - 2 | Failed to split string
 * - 3 | String doesn't include deliminator
 */
static int address_and_port_split(char** address, int* port, const char* string)
{
  if(!string) return 1;

  char* string_copy = strdup(string);

  char* token;

  // 1. Try to get address part of string
  if(!(token = strtok(string_copy, ":")))
  {
    free(string_copy);

    return 2;
  }

  if(address) *address = strdup(token);

  // 2. Try to get port part of string
  if(!(token = strtok(NULL, ":")))
  {
    free(string_copy);

    free(*address);

    return 3;
  }

  if(port) *port = atoi(token);

  free(string_copy);

  return 0;
}

/*
 *
 */
static int string_split(char*** strings, size_t* count, const char* string, const char* delim)
{
  if(!strings || !count || !string || !delim) return 1;

  char* string_copy = strdup(string);

  char* token = strtok(string_copy, delim);

  *strings = NULL;
  *count = 0;

  while(token)
  {
    *strings = realloc(*strings, sizeof(char*) * (*count + 1));

    if(!(*strings)) break;

    
    (*strings)[*count] = strdup(token);

    (*count)++;


    token = strtok(NULL, delim);
  }

  free(string_copy);

  if(!(*strings)) return 2;

  return 0;
}

typedef struct
{
  char* name;
  char* address;
  int   port;
} room_t;

/*
 *
 */
static int line_room_get(room_t* room, char* line)
{
  printf("Line: (%s)\n", line);

  char* token;

  if(!(token = strtok(line, ",")))
  {
    return 1;
  }

  room->name = strdup(token);

  if(!(token = strtok(NULL, ",")))
  {
    free(room->name);

    return 2;
  }

  if(address_and_port_split(&room->address, &room->port, token) != 0)
  {
    free(room->name);

    return 3;
  }

  return 0;
}

/*
 * Get a list of registered rooms
 *
 * RETURN (int status)
 * - 0 | Success
 * - 1 | Fail
 */
static int rooms_get(room_t** rooms, size_t* count)
{
  if(!rooms || !count) return 1;


  // 1. Read file with registered rooms
  size_t file_size = dir_file_size_get("../assets", "rooms.csv");

  char* buffer = malloc(sizeof(char) * (file_size + 1));

  if(dir_file_read(buffer, file_size, "../assets", "rooms.csv") == 0)
  {
    free(buffer);

    printf("Failed to read rooms file\n");

    return 1;
  }

  buffer[file_size] = '\0';


  // 2. Split file into seperate lines
  char** lines;

  if(string_split(&lines, count, buffer, "\n") != 0)
  {
    free(buffer);

    printf("Failed to split lines\n");

    return 2;
  }


  // 3. Allocate memory and populate rooms array
  *rooms = malloc(sizeof(room_t) * *count);

  for(size_t index = 0; index < *count; index++)
  {
    line_room_get(*rooms + index, lines[index]);
  }


  // 4. Free all the temporary memory
  for(size_t index = 0; index < *count; index++)
  {
    free(lines[index]);
  }

  free(lines);

  free(buffer);

  return 0;
}

/*
 *
 */
static int room_line_get(char* line, room_t room)
{
  sprintf(line, "%s, %s:%d", room.name, room.address, room.port)
}

/*
 * 
 */
static int rooms_store(room_t* rooms, size_t count)
{
  char** lines = malloc(sizeof(char*) * count);

  for(size_t index = 0; index < count; index++)
  {
    
  }
  return 1;
}

/*
 *
 */
static void room_free(room_t* room)
{
  if(!room) return;

  if(room->name) free(room->name);

  if(room->address) free(room->address);
}

/*
 *
 */
static void rooms_free(room_t** rooms, size_t count)
{
  if(!(*rooms)) return;

  for(size_t index = 0; index < count; index++)
  {
    room_free(*rooms + index);
  }

  free(*rooms);

  *rooms = NULL;
}

/*
 *
 */
static room_t* name_room_get(room_t* rooms, size_t count, const char* name)
{
  for(size_t index = 0; index < count; index++)
  {
    room_t* room = &rooms[index];

    if(strcmp(room->name, name) == 0) return room;
  }

  return NULL;
}

/*
 *
 */
static void address_and_port_store(char* address, int port, char* name)
{
  room_t room;

  room.name = strdup(name);

  room.address = strdup(address);

  room.port = port;


  // 1. Get all registered rooms
  room_t* rooms;
  size_t  count;

  if(rooms_get(&rooms, &count) != 0)
  {
    return 1;
  }


  size_t index;

  for(index = 0; index < count; index++)
  {
    if(strcmp(rooms[index].name, name) == 0)
    {
      room_free(rooms + index);

      rooms[index] = room;
      
      break;
    }
  }

  if(index == count)
  {
    rooms = realloc(sizeof(room_t) * (count + 1));

    if(!rooms)
    {
      printf("Failed to realloc rooms\n");

      return 2;
    }

    rooms[count] = room;

    count++;
  }

  rooms_store(rooms, count);

  return 0;
}

/*
 * Lookup string in lookup table to get address and port
 *
 * RETURN (int status)
 * - 0 | Success
 * - 1 | Fail
 */
static int address_and_port_lookup(char** address, int* port, const char* string)
{
  // 1. Get all registered rooms
  room_t* rooms;
  size_t  count;

  if(rooms_get(&rooms, &count) != 0)
  {
    return 1;
  }

  // Try to get room with matching name
  room_t* room = name_room_get(rooms, count, string);

  if(!room)
  {
    rooms_free(&rooms, count);

    return 1;
  }

  // Allocate memory and assign port and address
  if(address && room->address)
  {
    *address = strdup(room->address);
  }

  if(port) *port = room->port;

  rooms_free(&rooms, count);

  return 0;
}

/*
 * Get address and port from string
 *
 * RETURN (int status)
 * - 0 | Failed to get address and port
 * - 1 | Looked up address and port
 * - 2 | Parsed address and port
 */
static int address_and_port_get(char** address, int* port, const char* string)
{
  if(address_and_port_lookup(address, port, string) == 0)
  {
    // printf("Looked up address and port\n");
    return 1;
  }
  else if(address_and_port_split(address, port, string) == 0)
  {
    // printf("Parsed address and port\n");
    return 2;
  }
  else return 0;
}

static struct argp argp = { options, opt_parse, args_doc, doc };

/*
 * This is the main function
 */
int main(int argc, char* argv[])
{
  argp_parse(&argp, argc, argv, 0, 0, &args);

  if(args.name)
  {
    printf("Name: %s\n", args.name);
  }

  if(args.arg_count > 0)
  {
    printf("Room: %s\n", args.args[0]);

    char* address;
    int   port;

    if(address_and_port_get(&address, &port, args.args[0]) != 0)
    {
      printf("Address: %s\n", address);
      printf("Port: %d\n", port);

      if(args.room)
      {
        printf("New name of room: %s\n", args.room);

        address_and_port_store(address, port, args.room);
      }

      free(address);
    }
    else
    {
      printf("bunker: No room was found\n");
    }
  }

  free(args.args);

  return 0;
}