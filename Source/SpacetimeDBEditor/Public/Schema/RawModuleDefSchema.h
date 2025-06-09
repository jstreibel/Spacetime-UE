#pragma once

// Spacetime Algebraic Type System
namespace SATS
{
    using FOptionalString = TOptional<FString>;
    
    enum class EBuiltinType : uint8
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
        I256,   // { "I256": [] }
        U256,   // { "U256": [] }
        F32,    // { "F32": [] }
        F64,    // { "F64": [] }
        String, // { "String": [] }
        Array,  // { "Array": AlgebraicType }
        Map,    // { "Map": {"key_ty": AlgebraicType, "ty": AlgebraicType, } 
        Invalid
    };
   
    enum class EType : uint8
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
        I256,     // { "I256": [] }
        U256,     // { "U256": [] }
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
    
    inline FString BuiltinTypeToString(EBuiltinType Type)
    {
        switch (Type)
        {
            case EBuiltinType::Bool:   return "Bool";
            case EBuiltinType::I8:     return "I8";
            case EBuiltinType::U8:     return "U8";
            case EBuiltinType::I16:    return "I16";
            case EBuiltinType::U16:    return "U16";
            case EBuiltinType::I32:    return "I32";
            case EBuiltinType::U32:    return "U32";
            case EBuiltinType::I64:    return "I64";
            case EBuiltinType::U64:    return "U64";
            case EBuiltinType::I128:   return "I128";
            case EBuiltinType::U128:   return "U128";
            case EBuiltinType::I256:   return "I256";
            case EBuiltinType::U256:   return "U256";
            case EBuiltinType::F32:    return "F32";
            case EBuiltinType::F64:    return "F64";
            case EBuiltinType::String: return "String";
            case EBuiltinType::Array:  return "Array";
            case EBuiltinType::Map:    return "Map";
            default:                   return "Invalid";
        }
    };

    inline FString TypeToString(EType Kind)
    {
        switch (Kind)
        {
            case EType::Product:  return "Product";
            case EType::Sum:      return "Sum";
            case EType::Ref:      return "Ref";
            
            case EType::Bool:     return "Bool";
            case EType::I8:       return "I8";
            case EType::U8:       return "U8";
            case EType::I16:      return "I16";
            case EType::U16:      return "U16";
            case EType::I32:      return "I32";
            case EType::U32:      return "U32";
            case EType::I64:      return "I64";
            case EType::U64:      return "U64";
            case EType::I128:     return "I128";
            case EType::U128:     return "U128";
            case EType::I256:     return "I256";
            case EType::U256:     return "U256";
            case EType::F32:      return "F32";
            case EType::F64:      return "F64";
            case EType::String:   return "String";

            case EType::Array:    return "Array";
            case EType::Map:      return "Map";
            
            default:               return "Invalid";
        }
    }
    
    inline EType StringToType(const FString& Kind)
    {
        if (Kind == "Product")      return EType::Product;
        if (Kind == "Sum")          return EType::Sum;
        if (Kind == "Ref")          return EType::Ref;
        
        if (Kind == "Bool")         return EType::Bool;
        if (Kind == "I8")           return EType::I8;
        if (Kind == "U8")           return EType::U8;
        if (Kind == "I16")          return EType::I16;
        if (Kind == "U16")          return EType::U16;
        if (Kind == "I32")          return EType::I32;
        if (Kind == "U32")          return EType::U32;
        if (Kind == "I64")          return EType::I64;
        if (Kind == "U64")          return EType::U64;
        if (Kind == "I128")         return EType::I128;
        if (Kind == "U128")         return EType::U128;
        if (Kind == "I256")         return EType::I256;
        if (Kind == "U256")         return EType::U256;
        if (Kind == "F32")          return EType::F32;
        if (Kind == "F64")          return EType::F64;
        if (Kind == "String")       return EType::String;
        
        if (Kind == "Array")        return EType::Array;
        if (Kind == "Map")          return EType::Map;
        
        return EType::Invalid;

    }

    /**
     * This inline function is a map from a String to a Kind.
     * It is not a TMap class because it should return Invalid
     * if the type is not mapped.
     * @param Kind 
     * @return 
     */
    inline EBuiltinType StringToBuiltinType(const FString& Kind)
    {
 
        if (Kind == "Bool")         return EBuiltinType::Bool;
        if (Kind == "I8")           return EBuiltinType::I8;
        if (Kind == "U8")           return EBuiltinType::U8;
        if (Kind == "I16")          return EBuiltinType::I16;
        if (Kind == "U16")          return EBuiltinType::U16;
        if (Kind == "I32")          return EBuiltinType::I32;
        if (Kind == "U32")          return EBuiltinType::U32;
        if (Kind == "I64")          return EBuiltinType::I64;
        if (Kind == "U64")          return EBuiltinType::U64;
        if (Kind == "I128")         return EBuiltinType::I128;
        if (Kind == "U128")         return EBuiltinType::U128;
        if (Kind == "I256")         return EBuiltinType::I256;
        if (Kind == "U256")         return EBuiltinType::U256;
        if (Kind == "F32")          return EBuiltinType::F32;
        if (Kind == "F64")          return EBuiltinType::F64;
        if (Kind == "String")       return EBuiltinType::String;
        if (Kind == "Array")        return EBuiltinType::Array;
        if (Kind == "Map")          return EBuiltinType::Map;
        
        return EBuiltinType::Invalid;

    }

    struct FAlgebraicType;
    
    using VBuiltinType = TVariant<
        bool,
        int8,  uint8,
        int16, uint16,
        int32, uint32,
        int64, uint64, 
        std::array<char,7>,    // I128 and U128?
        // , int128, uint128
        float, double,
        FString
        // TODO: Resolve below: TMap and TArray need sizeof<FAlgebraicType>,
        // but here FAlgebraicType is forward declared. 
        // TArray<struct FAlgebraicType>,
        // TMap<struct FAlgebraicType, struct FAlgebraicType>
    >;
    
    struct FBuiltinType
    {
        EBuiltinType Tag = EBuiltinType::Invalid;
        VBuiltinType Value;
        
		/*bool Bool;
        int8 Int8;
  		uint8 UInt8;
        int16 Int16;
		uint16 UInt16;
        int32 Int32;
		uint32 UInt32;
		int64 Int64;
        uint64 UInt64;
        std::array<char,7> I128;
        std::array<char,7> U128;
        float F32;
        double F64;
        FString String;
        // TODO: Resolve below: TMap and TArray need sizeof<FAlgebraicType>,
        // but here FAlgebraicType is forward declared. 
        // , TArray<struct FAlgebraicType>
        // , TMap<struct FAlgebraicType, struct FAlgebraicType>
        */
    };
    
    struct FRefType {
        uint32 Index;  // e.g. "other_module.SomeType"
    };

    struct FProductType {
        
        struct FField
        {
            FOptionalString Name;
            TSharedPtr<FAlgebraicType> AlgebraicType;
        };
        TArray<FField> Elements;
    };

    struct FSumType {
        FString Tag;
        TMap<FOptionalString, TSharedPtr<FAlgebraicType>> Options;
    };

    struct FAlgebraicType {
        EType    Tag;
        FProductType Product;    // valid if kind==Product
        FSumType     Sum;        // valid if kind==Sum
        FBuiltinType Builtin;    // valid if kind==Builtin: e.g. "Int", "String"
        FRefType     Ref;        // valid if kind==Ref
    };
    
    // --- TypeSpace and TypeEntry ---
    struct FTypespace {
        struct FAlgebraicDef { FString Name; FAlgebraicType AlgebraicType; };
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
        FSumType Lifecycle;
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
        struct FParam { FOptionalString Name; FAlgebraicType Type; };
        // struct FReturnType { FString Tag; TSharedPtr<FJsonValue> Payload; };
        FString             Name;
        TArray<FParam>      Params;
        // FReturnType         ReturnType;
        FSumType            Lifecycle;
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