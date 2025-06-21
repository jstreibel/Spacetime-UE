#pragma once
#include "Math/BigInt.h"

// Spacetime Algebraic Type System
namespace SpacetimeDB
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
        I128,
        U128,
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
        I128,
        U128,
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

    inline bool IsBuiltIn(const EType &Type) { return Type < EType::Invalid; }

    inline bool IsBuiltInAdded(const EType &Type)
    {
        switch (Type)
        {
            case EType::I256:
            case EType::U256:
            case EType::I128:
            case EType::U128:
                return true;
        
            default:
                return false;
        }
    }
    
    inline bool HasNativeUnrealRepresentation(const EType& Type)
    {
        switch (Type)
        {
            case EType::Bool:
            case EType::I8:
            case EType::U8:
            case EType::I16:
            case EType::U16:
            case EType::I32:
            case EType::U32:
            case EType::I64:
            case EType::U64:
            // case EType::I128:
            // case EType::U128:
            // case EType::I256:
            // case EType::U256:
            case EType::F32:
            case EType::F64:
            case EType::String:
                return true;
        
            default:
                return false;
        }
    }

    
    inline bool IsNativeBlueprintSupported(const EType& Type)
    {
        switch (Type)
        {
            case EType::Bool:
            // case EType::I8:
            case EType::U8:
            // case EType::I16:
            // case EType::U16:
            case EType::I32:
            // case EType::U32:
            case EType::I64:
            // case EType::U64:
            // case EType::I128:
            // case EType::U128:
            // case EType::I256:
            // case EType::U256:
            case EType::F32:
            case EType::F64:
            case EType::String:
                return true;

            default:
                return false;
        }
    }
    
    inline FString BuiltinTypeToString(const EBuiltinType Type)
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
            case EBuiltinType::Invalid: return "Invalid";
        }

        UE_LOG(LogTemp, Warning, TEXT("Undealt with Sats BuiltIn type"));
        return "<unknown>";
    };

    inline FString TypeToString(const EType Kind)
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

    enum EMappingOption : uint8
    {
        MapToUnrealAvailableReflected,
        MapToUnrealNativeRepresentation,
    };
    
    inline FString MapBuiltinToUnreal(const FString& BuiltinName, const EMappingOption Option)
    {
        const auto bNative = Option == MapToUnrealNativeRepresentation;
        
        if (BuiltinName == "Bool")         return "bool";
        if (BuiltinName == "I8")           return bNative ? "int8"    : "uint8";    // Unreal does not reflect int8
        if (BuiltinName == "U8")           return "uint8"; 
        if (BuiltinName == "I16")          return bNative ? "int16"   : "int32";    // Unreal does not reflect int16
        if (BuiltinName == "U16")          return bNative ? "uint16"  : "int32";    // Unreal does not reflect uint16
        if (BuiltinName == "I32")          return "int32"; 
        if (BuiltinName == "U32")          return bNative ? "uint32"  : "int32";    // Unreal does not reflect uint32
        if (BuiltinName == "I64")          return "int64";
        if (BuiltinName == "U64")          return bNative ? "uint64"  : "int64";    // Unreal does not reflect uint64
        if (BuiltinName == "I128")         return bNative ? "int128"  : "FInt256";  // Unreal already has an FInt128 type (unfortunately, useless for us), so we use FInt256 instead. Notice that overflows will be harder with this type than the 128bit one.
        if (BuiltinName == "U128")         return bNative ? "uint128" : "FUInt128"; // These are hand-added USTRUCTs
        if (BuiltinName == "I256")         return bNative ? "int256"  : "FInt256";  // These are hand-added USTRUCTs
        if (BuiltinName == "U256")         return bNative ? "uint256" : "FUInt256"; // These are hand-added USTRUCTs
        if (BuiltinName == "F32")          return "float";
        if (BuiltinName == "F64")          return "double";
        if (BuiltinName == "String")       return "FString";
        if (BuiltinName == "Array")        return "// TArray<...>"; // TODO: Spacetime BuiltIn Arrays and Maps!!!
        if (BuiltinName == "Map")          return "// TMap<...>";
    
        return FString::Printf(TEXT("// unknown SATS BuiltIn '%s'"), *BuiltinName);
    }
    
    inline bool IsBlueprintSupported(const EType& InType)
    {
        if (InType == EType::I8)           return false;
        if (InType == EType::I16)          return false;
        if (InType == EType::U16)          return false;
        if (InType == EType::U32)          return false;
        if (InType == EType::U64)          return false;
        if (InType == EType::I128)         return false;
        if (InType == EType::U128)         return false;
        if (InType == EType::I256)         return false;
        if (InType == EType::U256)         return false;

        return true;
    }

    /**
     * 
     * @param Type The Spacetime Algebraic Type (including al subtypes of BuiltIn type)
     * @return 'True` if the type is a BuiltIn type AND has a native representation in Unreal, including non-reflected and implemented types.
     *         'False' if the type is a BuiltIn type and it has no native representation in Unreal.
     *         'False' if the type is not a BuiltIn type.
     *
     */
    /*
    inline bool IsBuiltinWithNativeRepresentation(const EType& Type)
    {
        const FString& TypeName = TypeToString(Type);
        const TSet<FString> BuiltinsWithNativeRepresentations = {
            "Bool",
            "I8",
            "U8",
            "I16",
            "U16",
            "I32",
            "U32",
            "I64",
            "U64",
            "I128",
            "U128",            
            "I256",
            "U256",
            "F32",
            "F64",
            "String",
            "Array",
            "Map"
        };
    
        return BuiltinsWithNativeRepresentations.Contains(TypeName);
    }
    */
    
    struct FAlgebraicType;
    
    using VBuiltinType = TVariant<
        bool,
        int8,  uint8,
        int16, uint16,
        int32, uint32,
        int64, uint64,
        // TODO: Add I128 and U128
        int256, /*uint256,*/
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
    };
    
    struct FRefType {
        uint32 Index;  // e.g. "other_module.SomeType"
    };
    
    struct FProductType {
        struct FField { FOptionalString Name; TSharedRef<FAlgebraicType> AlgebraicType; };
        TArray<FField> Elements;        
    };

    struct FSumType {        
        struct FVariant {FOptionalString Tag; TSharedRef<FAlgebraicType> AlgebraicType;};
        TArray<FVariant> Options;
    };

    struct FAlgebraicType {
        EType    Type;
        FProductType Product;    // valid if Type==Product
        FSumType     Sum;        // valid if Type==Sum
        FBuiltinType Builtin;    // valid if Type==Builtin: e.g. "Int", "String"
        FRefType     Ref;        // valid if Type==Ref
    };
    
    // --- TypeSpace and TypeEntry ---
    struct FTypespace {
        TArray<FAlgebraicType> TypeEntries;
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

    struct FRlsPolicy {
        FString Name;
        FString Using_Expr;
    };

    inline TArray<FExportedType> SortExportedTypesByRef(const TArray<FExportedType>& ExportedTypes)
    {
        TArray<FExportedType> SortedRefs = ExportedTypes;
        Algo::Sort(SortedRefs, [](const SpacetimeDB::FExportedType& EntryA, const  SpacetimeDB::FExportedType& EntryB)
        {
            return EntryA.TypeRef < EntryB.TypeRef;
        });

        return SortedRefs;
    }
    
    // Top‐level module definition
    struct FRawModuleDef {
        FTypespace Typespace;
        TArray<struct FTableDef>        Tables;
        TArray<struct FReducerDef>      Reducers;
        TArray<struct FExportedType>    ExportedTypes;
        TArray<struct FMiscExport>      MiscExports;
        TArray<struct FRlsPolicy>       RowLevelSecurity;
                
        TArray<FExportedType> GetRefSortedExportedTypes() const
        {
            return SortExportedTypesByRef(ExportedTypes);
        }
    };

    
}