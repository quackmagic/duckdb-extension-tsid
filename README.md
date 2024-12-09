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
-- Generate a new time-sortable ID (requires any input string)
SELECT tsid('any');
```

### Working with Timestamps

Extract the timestamp from a TSID:
```sql
-- Extract timestamp from a TSID
SELECT tsid_to_timestamp(tsid('any'));

-- Verify TSID ordering
SELECT 
    tsid('a') as id1,
    tsid('b') as id2,
    tsid_to_timestamp(id1) as ts1,
    tsid_to_timestamp(id2) as ts2
FROM range(1);
```

### Bulk Generation

Generate multiple TSIDs:
```sql
-- Generate 1000 unique TSIDs
D SELECT tsid(random()::VARCHAR) as unique_id 
  FROM range(10);
```

### Features
- Time-sortable: TSIDs are chronologically sortable by string comparison
- Unique: Each ID is guaranteed to be unique
- Timestamp extraction: Get the creation time of any TSID
- High performance: Suitable for bulk generation
