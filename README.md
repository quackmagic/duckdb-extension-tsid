<img src="https://github.com/user-attachments/assets/46a5c546-7e9b-42c7-87f4-bc8defe674e0" width=250 />

# DuckDB TSID Community Extension 
This very experimental extension generates Time-Sorted Unique Identifiers (TSID)

> Experimental: USE AT YOUR OWN RISK!

### 📦 Installation
```sql
INSTALL tsid FROM community;
LOAD tsid;
```

#### Table Functions
- `tsid()`

### Example
```sql
SELECT * FROM tsid();
```
