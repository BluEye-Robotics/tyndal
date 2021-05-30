#pragma once
#include "shmem.h"
#include <tyndall/meta/strval.h>

#ifndef IPC_SHMEM_PREFIX
//                              echo ipc | sha1sum  # more unique shared memory names reduce the chance of name collisions
#define IPC_SHMEM_PREFIX "ipc" "1ef42bc4e0bbfeb0ac34bc3642732768cf6f77b7"
#endif

#ifdef __cplusplus

#include "seq_lock.h"
#include <tyndall/meta/strval.h>

// Ipc ids are unique except for leading slashes.

template<typename STRING>
using ipc_id_remove_leading_slashes = decltype(STRING::template remove_leading<'/'>());

template<typename STRING>
using ipc_id_replace_slashes_with_underscores = decltype(STRING::template replace<'/', '_'>());

template<typename ID>
using ipc_id_convert =
  decltype(create_strval(IPC_SHMEM_PREFIX)
  + "_"_strval
  + to_strval<ipc_id_remove_leading_slashes<ID>::hash()>{} // hash id to prevent name clash between f.ex. /my/topic and my_topic
  + "_"_strval
  + ipc_id_replace_slashes_with_underscores<ipc_id_remove_leading_slashes<ID>>{}); // switch slashes with underscores

// Standard, lazy ipc methods, templated on storage type and id,
// so that the transport will be initiated only on calls with new combinations of storage type and id.
#define ipc_write(entry, id) ipc_lazy_write(entry, id ## _strval)
#define ipc_read(entry, id) ipc_lazy_read(entry, id ## _strval)

template<typename STORAGE, typename ID>
using ipc_writer = shmem_data<seq_lock<STORAGE>, SHMEM_WRITE, ipc_id_convert<ID>>;
template<typename STORAGE, typename ID>
using ipc_reader = shmem_data<seq_lock<STORAGE>, SHMEM_READ, ipc_id_convert<ID>>;

template<typename STORAGE, typename ID>
int ipc_lazy_write(const STORAGE& entry, ID)
{
  static ipc_writer<STORAGE, ID> writer;

  return writer.write(entry);
}

template<typename STORAGE, typename ID>
int ipc_lazy_read(STORAGE& entry, ID)
{
  static ipc_reader<STORAGE, ID> reader;

  return reader.read(entry);
}

// Ipc methods with id specified at runtime.
// These methods are templated on storage type only,
// and need to explicitly keep track of lazy initialization of transport.
#define ipc_rtid_write(entry, id) ipc_rtid_lazy_get<ipc_rtid_writer, std::remove_cv_t<std::remove_reference_t<decltype(entry)>>>(id).write(entry)
#define ipc_rtid_read(entry, id) ipc_rtid_lazy_get<ipc_rtid_reader, std::remove_cv_t<std::remove_reference_t<decltype(entry)>>>(id).read(entry)

#include <string>
#include <vector>

template<typename STORAGE>
using ipc_rtid_writer = shmem_data<seq_lock<STORAGE>, SHMEM_WRITE>;
template<typename STORAGE>
using ipc_rtid_reader = shmem_data<seq_lock<STORAGE>, SHMEM_READ>;

template<template<typename>typename TRANSPORT, typename STORAGE>
TRANSPORT<STORAGE>& ipc_rtid_lazy_get(const char* id)
{
  // remove leading slashes
  while (*id == '/')
    ++id;

  std::string converted_id = IPC_SHMEM_PREFIX "_" + std::to_string(hash_fnv1a_32(id, strlen(id))) + "_" + std::string{id};
  // replace slash with underscore
  for (char& c : converted_id)
    if (c == '/')
      c = '_';

  struct registry_item
  {
    std::string id;
    TRANSPORT<STORAGE> transport;
  };
  static std::vector<registry_item> registry;

  for (auto& item : registry)
    if (item.id == converted_id)
      return item.transport;

  registry.push_back(registry_item{ .id = converted_id, .transport = TRANSPORT<STORAGE>(converted_id.c_str()) });
  return registry.back().transport;
}

#endif

int ipc_cleanup()
{
  return shmem_unlink_all(IPC_SHMEM_PREFIX);
}
