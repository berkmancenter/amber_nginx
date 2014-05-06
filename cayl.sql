CREATE TABLE cayl_activity (
  id     TEXT,
  date   INTEGER,
  views  INTEGER,
  PRIMARY KEY  (id)
);

CREATE TABLE cayl_cache (
  id         TEXT NOT NULL,
  url        TEXT NOT NULL,
  location   TEXT NOT NULL,
  date       INTEGER NOT NULL,
  type       TEXT NOT NULL,
  size       INTEGER NOT NULL,
  PRIMARY KEY  (id)
);

CREATE TABLE cayl_check (
  id             TEXT NOT NULL,
  url            TEXT NOT NULL,
  status         INTEGER,
  last_checked   INTEGER,
  next_check     INTEGER,
  PRIMARY KEY  (id)
);

CREATE TABLE cayl_queue (
  url            TEXT NOT NULL,
  created        INTEGER,
  lock           INTEGER,
  PRIMARY KEY(url)
);

CREATE TABLE cayl_exclude (
  url            TEXT NOT NULL,
  created        INTEGER,
  PRIMARY KEY(url)
);
