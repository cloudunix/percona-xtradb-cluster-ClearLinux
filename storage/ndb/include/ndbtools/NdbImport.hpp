/*
   Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#ifndef NDB_IMPORT_HPP
#define NDB_IMPORT_HPP

// STL
#include <map>

class NdbOut;
class Ndb_cluster_connection;

class NdbImport {
public:
  NdbImport();
  ~NdbImport();

  // csv spec

  struct OptCsv {
    OptCsv();
    enum Mode {
      ModeInput = 1,
      ModeOutput = 2
    };
    const char* m_fields_terminated_by;
    const char* m_fields_enclosed_by;
    const char* m_fields_optionally_enclosed_by;
    const char* m_fields_escaped_by;
    const char* m_lines_terminated_by;
  };

  // opt

  struct Opt {
    Opt();
    uint m_connections;
    const char* m_database;
    const char* m_state_dir;
    bool m_keep_state;
    const char* m_table;
    const char* m_input_type;
    const char* m_input_file;
    uint m_input_workers;
    const char* m_output_type;
    uint m_output_workers;
    uint m_db_workers;
    uint m_ignore_lines;
    uint m_max_rows;
    const char* m_result_file;
    const char* m_reject_file;
    const char* m_rowmap_file;
    const char* m_stats_file;
    bool m_continue;
    bool m_resume;
    uint m_monitor;
    uint m_ai_prefetch_sz;
    uint m_ai_increment;
    uint m_ai_offset;
    bool m_no_asynch;
    bool m_no_hint;
    uint m_pagesize;
    uint m_pagecnt;
    uint m_rowbatch;
    uint m_rowbytes;
    uint m_opbatch;
    uint m_opbytes;
    uint m_polltimeout;
    uint m_temperrors;
    uint m_tempdelay;
    uint m_idlesleep;
    uint m_idlespin;
    uint m_rejects;
    // csv options
    OptCsv m_optcsv;
    // debug options
    uint m_verbose;
    bool m_abort_on_error;
    const char* m_errins_type;
    uint m_errins_delay;
  };

  int set_opt(const Opt& opt);

  // connect

  // pre-created connections (not used by ndb_import)
  int set_connections(int cnt, Ndb_cluster_connection** connections);
  int do_connect();
  void do_disconnect();

  // table

  int add_table(const char* database, const char* table, uint& tabid);
  int set_tabid(uint tabid);

  // job

  struct Job;
  struct Team;
  struct Error;

  struct JobStatus {
    enum Status {
      Status_null = 0,
      Status_created,
      Status_starting,
      Status_running,
      Status_success,
      Status_error,
      Status_fatal
    };
  };

  struct JobStats {
    JobStats();
    uint64 m_rows;
    uint64 m_reject;
    uint m_temperrors;  // sum of values from m_errormap
    std::map<uint, uint> m_errormap;
    uint64 m_runtime;
    uint64 m_rowssec;
    uint64 m_utime;
    uint64 m_stime;
  };

  struct Job {
    Job(NdbImport& imp);
    ~Job();
    int do_create();
    int do_start();
    int do_stop();      // ask to stop before ready
    int do_wait();
    void do_destroy();
    bool has_error() const;
    const Error& get_error() const;
    NdbImport& m_imp;
    uint m_jobno;
    JobStatus::Status m_status;
    const char* m_str_status;
    JobStats m_stats;
    uint m_teamcnt;
    Team** m_teams;
    // update status of job and all teams
    void get_status();
  };

  struct TeamStatus {
    enum Status {
      Status_null = 0
    };
  };

  struct Team {
    Team(const Job& job, uint teamno);
    const char* get_name();
    bool has_error() const;
    const Error& get_error() const;
    const Job& m_job;
    const uint m_teamno;
    // snapshot or final status
    enum Status {
      Status_null = 0
    };
    TeamStatus::Status m_status;
    const char* m_str_status;
  };

  static const char* g_str_status(JobStatus::Status status);
  static const char* g_str_status(TeamStatus::Status status);

  // error

  struct Error {
    enum Type {
      Type_noerror = 0,
      Type_gen = 1,
      Type_usage = 2,
      Type_alloc = 3,
      Type_mgm = 4,
      Type_con = 5,
      Type_ndb = 6,
      Type_os = 7,
      Type_data = 8
    };
    Error();
    const char* gettypetext() const;
    Type type;
    int code;
    int line;
    char text[1024];
  };

  bool has_error() const;
  const Error& get_error() const;
  friend class NdbOut& operator<<(NdbOut&, const Error&);

  // stop all jobs (crude way to handle signals)
  static void set_stop_all();

private:
  friend class NdbImportImpl;
  NdbImport(class NdbImportImpl& impl);
  class NdbImportImpl& m_impl;
};

#endif