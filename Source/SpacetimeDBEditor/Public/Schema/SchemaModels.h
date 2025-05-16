#pragma once

struct FColumnSchema
{
	FString Name;           // field name
	FString JsonType;		// raw JSON type
	FString UEType;         // e.g. "int32", "FString", "TArray<FString>"
	bool bIsOptional;       // for nullable types
	bool bIsPrimaryKey = false; // true if this column is part of the primary key
};

struct SATSType;

struct FTableSchema
{
	FString TableName;                // e.g. "ChatMessage"
	// SATSType type;
	TArray<FColumnSchema> Columns;       // column definitions
	bool bIsPublic;                   // from table_access
};

struct FReducerSchema
{
	FString ReducerName;              // e.g. "SendMessage"
	TArray<FColumnSchema> Params;        // input args
	// (optional) return type info
};