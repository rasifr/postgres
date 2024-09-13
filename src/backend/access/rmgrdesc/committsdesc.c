/*-------------------------------------------------------------------------
 *
 * committsdesc.c
 *	  rmgr descriptor routines for access/transam/commit_ts.c
 *
 * Portions Copyright (c) 1996-2024, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  src/backend/access/rmgrdesc/committsdesc.c
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "access/commit_ts.h"


void
commit_ts_desc(StringInfo buf, XLogReaderState *record)
{
	char	   *rec = XLogRecGetData(record);
	uint8		info = XLogRecGetInfo(record) & ~XLR_INFO_MASK;

	if (info == COMMIT_TS_ZEROPAGE)
	{
		int64		pageno;

		memcpy(&pageno, rec, sizeof(pageno));
		appendStringInfo(buf, "%lld", (long long) pageno);
	}
	else if (info == COMMIT_TS_TRUNCATE)
	{
		xl_commit_ts_truncate *trunc = (xl_commit_ts_truncate *) rec;

		appendStringInfo(buf, "pageno %lld, oldestXid %u",
						 (long long) trunc->pageno, trunc->oldestXid);
	}
	else if (info == COMMIT_TS_SUBTRANS_TS)
	{
		SubTransactionCommitTsEntry	*entry = (SubTransactionCommitTsEntry *)rec;

		appendStringInfo(buf, "xid %u, time %ld, nodeid %d",
						 entry->xid, entry->time, entry->nodeid);
	}
}

const char *
commit_ts_identify(uint8 info)
{
	switch (info)
	{
		case COMMIT_TS_ZEROPAGE:
			return "ZEROPAGE";
		case COMMIT_TS_TRUNCATE:
			return "TRUNCATE";
		case COMMIT_TS_SUBTRANS_TS:
			return "SUBTRANS_TS";
		default:
			return NULL;
	}
}
