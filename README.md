<img src="https://github.com/user-attachments/assets/46a5c546-7e9b-42c7-87f4-bc8defe674e0" width=250 />

# DuckDB TSID Community Extension 
This extension adds support for Time-Sorted Unique Identifiers (TSID)

> Experimental: USE AT YOUR OWN RISK!

### 📦 Installation
```sql
INSTALL tsid FROM community;
LOAD tsid;
```

### Usage

Generate a new TSID:
```sql
-- Generate a new time-sortable ID (accepts an optional seed string)
D SELECT tsid();
┌──────────────────────────────────┐
│              tsid()              │
│             varchar              │
├──────────────────────────────────┤
│ 675716e86985495e9cf575f0b9c4a8db │
└──────────────────────────────────┘
```

### Working with Timestamps

Extract the timestamp from a TSID:
```sql
-- Extract timestamp from a TSID
D SELECT tsid_to_timestamp('675716e86985495e9cf575f0b9c4a8db');
┌───────────────────────────────────────────────────────┐
│ tsid_to_timestamp('675716e86985495e9cf575f0b9c4a8db') │
│                       timestamp                       │
├───────────────────────────────────────────────────────┤
│ 2024-12-09 16:12:24.44259                             │
└───────────────────────────────────────────────────────┘

-- Verify TSID ordering
D SELECT 
      a.id as id1,
      b.id as id2,
      tsid_to_timestamp(a.id) as ts1,
      tsid_to_timestamp(b.id) as ts2
  FROM 
      (SELECT tsid('a') as id FROM range(1)) a,
      (SELECT tsid('b') as id FROM range(1)) b;
┌──────────────────────────────────┬──────────────────────────────────┬────────────────────────────┬────────────────────────────┐
│               id1                │               id2                │            ts1             │            ts2             │
│             varchar              │             varchar              │         timestamp          │         timestamp          │
├──────────────────────────────────┼──────────────────────────────────┼────────────────────────────┼────────────────────────────┤
│ 6757176b9d4640b299be06ff79ed1373 │ 6757176b9d704dcf8a1dce8bb6da51bf │ 2024-12-09 16:14:35.659653 │ 2024-12-09 16:14:35.660354 │
└──────────────────────────────────┴──────────────────────────────────┴────────────────────────────┴────────────────────────────┘
```

### Bulk Generation

Generate multiple TSIDs:
```sql
-- Generate 100 sequential unique TSIDs
D SELECT tsid(random()::VARCHAR) as unique_id 
  FROM range(100);
```

### Features
- Time-sortable: TSIDs are chronologically sortable by string comparison
- Unique: Each ID is guaranteed to be unique
- Timestamp extraction: Get the creation time of any TSID
- High performance: Suitable for bulk generation
