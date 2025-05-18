#pragma once

// Spacetime Algebraic Type System
namespace SATS
{
    enum class EBuiltinKind : uint8
    {
        Bool,   // { "Bool": [] }
        I8,     // { "I8": [] }
        U8,     // { "U8": [] }
        I16,    // { "I16": [] }
        U16,    // { "U16": [] }
        I32,    // { "I32": [] }
        U32,    // { "U32": [] }
        I64,    // { "I64": [] }
        U64,    // { "U64": [] }
        I128,   // { "I128": [] }
        U128,   // { "U128": [] }
        F32,    // { "F32": [] }
        F64,    // { "F64": [] }
        String, // { "String": [] }
        Array,  // { "Array": AlgebraicType }
        Map,    // { "Map": {"key_ty": AlgebraicType, "ty": AlgebraicType, } 
        Invalid
    };

    
    enum class EKind : uint8
    {        
        // Don't change this order
        Bool,     // { "Bool": [] }
        I8,       // { "I8": [] }
        U8,       // { "U8": [] }
        I16,      // { "I16": [] }
        U16,      // { "U16": [] }
        I32,      // { "I32": [] }
        U32,      // { "U32": [] }
        I64,      // { "I64": [] }
        U64,      // { "U64": [] }
        I128,     // { "I128": [] }
        U128,     // { "U128": [] }
        F32,      // { "F32": [] }
        F64,      // { "F64": [] }
        String,   // { "String": [] }
 
        Array,    // { "Array": AlgebraicType }
        Map,      // { "Map": {"key_ty": AlgebraicType, "ty": AlgebraicType, }

        Invalid,

        Product,  // { "Product": {"elements": []}
        Sum,      // { "Sum":     {"variants": []}
        Ref,      // { ? }
    };
    
    using VBuiltinValue = std::variant<
    bool,
    int8,  uint8,
    int16, uint16,
    int32, uint32,
    int64, uint64,
    std::array<char,7>,    // for I128
    std::array<char,7>,    // for U128
    float, double,
    FString,
    TArray<struct FAlgebraicKind>>;


    inline FString BuiltinKindToString(EBuiltinKind Kind)
    {
        switch (Kind)
        {
            case EBuiltinKind::Bool:   return "Bool";
            case EBuiltinKind::I8:     return "I8";
            case EBuiltinKind::U8:     return "U8";
            case EBuiltinKind::I16:    return "I16";
            case EBuiltinKind::U16:    return "U16";
            case EBuiltinKind::I32:    return "I32";
            case EBuiltinKind::U32:    return "U32";
            case EBuiltinKind::I64:    return "I64";
            case EBuiltinKind::U64:    return "U64";
            case EBuiltinKind::I128:   return "I128";
            case EBuiltinKind::U128:   return "U128";
            case EBuiltinKind::F32:    return "F32";
            case EBuiltinKind::F64:    return "F64";
            case EBuiltinKind::String: return "String";
            case EBuiltinKind::Array:  return "Array";
            case EBuiltinKind::Map:    return "Map";
            default:                   return "Invalid";
        }
    };

    inline FString KindToString(EKind Kind)
    {
        switch (Kind)
        {
            case EKind::Product:  return "Product";
            case EKind::Sum:      return "Sum";
            case EKind::Ref:      return "Ref";
            
            case EKind::Bool:     return "Bool";
            case EKind::I8:       return "I8";
            case EKind::U8:       return "U8";
            case EKind::I16:      return "I16";
            case EKind::U16:      return "U16";
            case EKind::I32:      return "I32";
            case EKind::U32:      return "U32";
            case EKind::I64:      return "I64";
            case EKind::U64:      return "U64";
            case EKind::I128:     return "I128";
            case EKind::U128:     return "U128";
            case EKind::F32:      return "F32";
            case EKind::F64:      return "F64";
            case EKind::String:   return "String";

            case EKind::Array:    return "Array";
            case EKind::Map:      return "Map";
            
            default:               return "Invalid";
        }
    }
    
    inline EKind StringToSatsKind(const FString& Kind)
    {
        if (Kind == "Product")      return EKind::Product;
        if (Kind == "Sum")          return EKind::Sum;
        if (Kind == "Ref")          return EKind::Ref;
        
        if (Kind == "Bool")         return EKind::Bool;
        if (Kind == "I8")           return EKind::I8;
        if (Kind == "U8")           return EKind::U8;
        if (Kind == "I16")          return EKind::I16;
        if (Kind == "U16")          return EKind::U16;
        if (Kind == "I32")          return EKind::I32;
        if (Kind == "U32")          return EKind::U32;
        if (Kind == "I64")          return EKind::I64;
        if (Kind == "U64")          return EKind::U64;
        if (Kind == "I128")         return EKind::I128;
        if (Kind == "U128")         return EKind::U128;
        if (Kind == "F32")          return EKind::F32;
        if (Kind == "F64")          return EKind::F64;
        if (Kind == "String")       return EKind::String;
        
        if (Kind == "Array")        return EKind::Array;
        if (Kind == "Map")          return EKind::Map;
        
        return EKind::Invalid;

    }

    /**
     * This inline function is a map from a String to a Kind.
     * It is not a TMap class because it should return Invalid
     * if the type is not mapped.
     * @param Kind 
     * @return 
     */
    inline EBuiltinKind StringToBuiltinKind(const FString& Kind)
    {
 
        if (Kind == "Bool")         return EBuiltinKind::Bool;
        if (Kind == "I8")           return EBuiltinKind::I8;
        if (Kind == "U8")           return EBuiltinKind::U8;
        if (Kind == "I16")          return EBuiltinKind::I16;
        if (Kind == "U16")          return EBuiltinKind::U16;
        if (Kind == "I32")          return EBuiltinKind::I32;
        if (Kind == "U32")          return EBuiltinKind::U32;
        if (Kind == "I64")          return EBuiltinKind::I64;
        if (Kind == "U64")          return EBuiltinKind::U64;
        if (Kind == "I128")         return EBuiltinKind::I128;
        if (Kind == "U128")         return EBuiltinKind::U128;
        if (Kind == "F32")          return EBuiltinKind::F32;
        if (Kind == "F64")          return EBuiltinKind::F64;
        if (Kind == "String")       return EBuiltinKind::String;
        if (Kind == "Array")        return EBuiltinKind::Array;
        if (Kind == "Map")          return EBuiltinKind::Map;
        
        return EBuiltinKind::Invalid;

    }
    
    struct FBuiltinKind
    {
        FBuiltinKind() : Tag(EBuiltinKind::Invalid), Value(0) { }
        EBuiltinKind Tag;
        VBuiltinValue Value; // Used in the case of Array or Map
    };    
    
    struct FRefKind {
        FString Path;  // e.g. "other_module.SomeType"
    };

    struct FProductKind {
        struct FField { FString Name; TSharedPtr<FAlgebraicKind> AlgebraicType; };
        TArray<FField> Elements;
    };

    struct FSumKind {
        FString Tag;
        TMap<FString, TSharedPtr<FAlgebraicKind>> Options;
    };

    struct FAlgebraicKind {
        EKind    Tag;
        FProductKind Product;    // valid if kind==Product
        FSumKind     Sum;        // valid if kind==Sum
        FBuiltinKind Builtin;    // valid if kind==Builtin: e.g. "Int", "String"
        FRefKind     Ref;        // valid if kind==Ref
    };
    
    // --- TypeSpace and TypeEntry ---
    struct FTypespace {
        struct FAlgebraicDef { FString Name; FAlgebraicKind AlgebraicKind; };
        TArray<FAlgebraicDef> TypeEntries;
    };
    
    // Top‐level module definition
    struct FRawModuleDef {
        FTypespace Typespace;
        TArray<struct FTableDef>        Tables;
        TArray<struct FReducerDef>      Reducers;
        TArray<struct FExportedType>    Types;
        TArray<struct FMiscExport>      MiscExports;
        TArray<struct FRLSPolicy>       RowLevelSecurity;
    };

    // TODO: struct FRefType   

    struct FSchedule {
        // represent as a SumType or custom struct
        FSumKind Lifecycle;
    };

    // --- Tables ---
    struct FTableDef {
        FString                       Name;
        int                           ProductTypeRef;
        TArray<FString>               PrimaryKey;
        TArray<struct FIndexDef>      Indexes;
        TArray<struct FConstraintDef> Constraints;
        TArray<struct FSequenceDef>   Sequences;
        FSchedule Schedule;
        FString                       TableType;
        FString                       TableAccess;
    };

    // --- Reducers ---
    struct FReducerDef {
        struct FReturnType { FString Tag; TSharedPtr<FJsonValue> Payload; };
        FString             Name;
        FProductKind        Params;
        FReturnType         ReturnType;
        FSumKind            Lifecycle;
    };

    struct FIndexDef { FString Name; TArray<FString> Columns; };
    struct FConstraintDef { FString Expr; };
    struct FSequenceDef { FString Name; int Start; };

    // --- Exported client‐side types ---
    struct FExportedType {
        struct FScopedName { TArray<FString> Scope; FString Name; };
        FScopedName        Name;
        int                TypeRef;
        bool               bCustomOrdering;
    };

    // --- Misc & RLS ---
    struct FMiscExport {
        FString Key;
        FString Value;
    };

    struct FRLSPolicy {
        FString Name;
        FString Using_Expr;
    };
}