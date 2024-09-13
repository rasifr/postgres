
/*
 * commit_ts.h
 *
 * PostgreSQL commit timestamp manager
 *
 * Portions Copyright (c) 1996-2024, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/access/commit_ts.h
 */
#ifndef COMMIT_TS_H
#define COMMIT_TS_H

#include "access/xlog.h"
#include "datatype/timestamp.h"
#include "replication/origin.h"
#include "storage/sync.h"

/*
 * pgEdge requires to override the CommitTimestampEntry for individual
 * subtransactions in the case that delta-apply needs to override losing
 * last-update-wins. spock_apply_heap will then perform this (rare)
 * update in a subtransaction so that this individual row gets its own
 * xmin. By overriding this xid's CommitTsData the row can retain its
 * original CommitTsData and not assume that of the overall replication
 * transaction.
 */
typedef struct SubTransactionCommitTsEntry
{
	TransactionId	xid;
	TimestampTz		time;
	RepOriginId		nodeid;
} SubTransactionCommitTsEntry;

extern PGDLLIMPORT bool track_commit_timestamp;

extern void TransactionTreeSetCommitTsData(TransactionId xid, int nsubxids,
										   TransactionId *subxids, TimestampTz timestamp,
										   RepOriginId nodeid);
extern bool TransactionIdGetCommitTsData(TransactionId xid,
										 TimestampTz *ts, RepOriginId *nodeid);
extern TransactionId GetLatestCommitTsData(TimestampTz *ts,
										   RepOriginId *nodeid);
extern void SubTransactionIdSetCommitTsData(TransactionId xid,
											TimestampTz ts,
											RepOriginId nodeid);

extern Size CommitTsShmemSize(void);
extern void CommitTsShmemInit(void);
extern void BootStrapCommitTs(void);
extern void StartupCommitTs(void);
extern void CommitTsParameterChange(bool newvalue, bool oldvalue);
extern void CompleteCommitTsInitialization(void);
extern void CheckPointCommitTs(void);
extern void ExtendCommitTs(TransactionId newestXact);
extern void TruncateCommitTs(TransactionId oldestXact);
extern void SetCommitTsLimit(TransactionId oldestXact,
							 TransactionId newestXact);
extern void AdvanceOldestCommitTsXid(TransactionId oldestXact);

extern int	committssyncfiletag(const FileTag *ftag, char *path);

/* XLOG stuff */
#define COMMIT_TS_ZEROPAGE		0x00
#define COMMIT_TS_TRUNCATE		0x10
#define COMMIT_TS_SUBTRANS_TS	0x20

typedef struct xl_commit_ts_set
{
	TimestampTz timestamp;
	RepOriginId nodeid;
	TransactionId mainxid;
	/* subxact Xids follow */
}			xl_commit_ts_set;

#define SizeOfCommitTsSet	(offsetof(xl_commit_ts_set, mainxid) + \
							 sizeof(TransactionId))

typedef struct xl_commit_ts_truncate
{
	int64		pageno;
	TransactionId oldestXid;
} xl_commit_ts_truncate;

#define SizeOfCommitTsTruncate	(offsetof(xl_commit_ts_truncate, oldestXid) + \
								 sizeof(TransactionId))

extern void commit_ts_redo(XLogReaderState *record);
extern void commit_ts_desc(StringInfo buf, XLogReaderState *record);
extern const char *commit_ts_identify(uint8 info);

#endif							/* COMMIT_TS_H */
