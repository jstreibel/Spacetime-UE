#pragma once

struct FFieldInfo
{
	FString Name;           // field name
	FString UEType;         // e.g. "int32", "FString", "TArray<FString>"
	bool bIsOptional;       // for nullable types
};

struct FTableSchema
{
	FString TableName;                // e.g. "ChatMessage"
	TArray<FFieldInfo> Fields;        // column definitions
	bool bIsPublic;                   // from table_access
};

struct FReducerSchema
{
	FString ReducerName;              // e.g. "SendMessage"
	TArray<FFieldInfo> Params;        // input args
	// (optional) return type info
};
