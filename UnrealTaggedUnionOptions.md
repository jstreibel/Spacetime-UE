# Unreal C++: Tagged Union Options

When you need Unreal reflection/serialization support (UPROPERTY/Blueprint), here are three patterns you can choose from:

---

## Option 1: Tag + All Fields in a USTRUCT

### Definition

```cpp
UENUM(BlueprintType)
enum class EMyTag : uint8
{
    None   UMETA(DisplayName="None"),
    TypeA  UMETA(DisplayName="Type A"),
    TypeB  UMETA(DisplayName="Type B"),
    TypeC  UMETA(DisplayName="Type C"),
};

USTRUCT(BlueprintType)
struct FMyVariant
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Variant")
    EMyTag Tag = EMyTag::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Variant")
    FTypeA A;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Variant")
    FTypeB B;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Variant")
    FTypeC C;
};
```

### Pros
- Full UPROPERTY/Blueprint/Editor integration  
- Zero additional boilerplate beyond enum and fields

### Cons
- Memory for *all* variant fields, even inactive ones  
- No built-in visitor API

---

## Option 2: Polymorphic USTRUCT using FInstancedStruct (UE5+)

### Definition

First, define a base struct and variant structs:

```cpp
USTRUCT(BlueprintType)
struct FMyVariantBase
{
    GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct FMyVariantA : public FMyVariantBase
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTypeA ValueA;
};

USTRUCT(BlueprintType)
struct FMyVariantB : public FMyVariantBase
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTypeB ValueB;
};

// ...and FMyVariantC similarly...
```

Then, in your container struct:

```cpp
#include "InstancedStruct/InstancedStruct.h"

USTRUCT(BlueprintType)
struct FMyVariant
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Variant",
              meta=(BaseStruct="MyModule.MyVariantBase", ExcludeBaseStruct))
    FInstancedStruct Payload;
};
```

### Pros
- Fully reflective polymorphism in a single property  
- Minimal memory waste

### Cons
- Requires UE5 StructUtils (`FInstancedStruct` support)

---

## Option 3: UObject Polymorphism with Instanced UPROPERTY

### Definition

Define an abstract base UObject and subclasses:

```cpp
UCLASS(Abstract, BlueprintType)
class UMyVariantValue : public UObject
{
    GENERATED_BODY()
};

UCLASS(BlueprintType)
class UMyVariantA : public UMyVariantValue
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTypeA ValueA;
};

UCLASS(BlueprintType)
class UMyVariantB : public UMyVariantValue
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTypeB ValueB;
};

// ...and UMyVariantC similarly...
```

Then, in your USTRUCT:

```cpp
USTRUCT(BlueprintType)
struct FMyVariant
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Instanced, Category="Variant")
    UMyVariantValue* Payload = nullptr;
};
```

### Pros
- Full UObject features: GC, Blueprintable methods, events  
- Extensible at runtime with methods and overrides

### Cons
- Heap allocation and GC overhead  
- More boilerplate than `FInstancedStruct`

---

## Choosing the Right Pattern

| Use Case                                    | Recommended Option                    |
|---------------------------------------------|---------------------------------------|
| Simple & few variant types                  | Option 1: Tag + Fields                |
| Many types & minimal memory footprint       | Option 2: FInstancedStruct polymorphism |
| Need full UObject features (methods, GC)    | Option 3: UObject + Instanced         |
