// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyMagicWidget.generated.h"


USTRUCT()
struct FSpellPoints
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<int32> Points;
};
/**
 * 
 */
UCLASS()
class MAGIC_API UMyMagicWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UMyMagicWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	virtual void SynchronizeProperties() override;

	/**
	* Should be called in OnPaint block
	**/
	UFUNCTION(BlueprintCallable, Category = MagicControl)
	void DrawMagicLines(FPaintContext Context);

	/**
	* Should be called in OnTouchStarted block
	*
	* @param TouchPosition	AbsoluteToViewport of Touch ScreenSpacePosition
	**/
	UFUNCTION(BlueprintCallable, Category = MagicControl)
	void StartLine(FVector2D TouchPosition);

	/**
	* Should be called in OnTouchMoved block
	*
	* @param TouchPosition  AbsoluteToViewport of Touch ScreenSpacePosition
	**/
	UFUNCTION(BlueprintCallable, Category = MagicControl)
	void MoveLine(FVector2D TouchPosition);

	/**
	* Should be called in OnTouchEnded block
	*
	**/
	UFUNCTION(BlueprintCallable, Category = MagicControl)
	void EndLine();

	FVector2D GetImagePosition(int32 ImageIndex);

	/*
	* Check if there is no recurring lines

	*/
	bool CheckPoints(int32 ImageIndex);

	FString CheckForSpell();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UBorder* MagicBorder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UCanvasPanel* MagicCanvas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UImage* Point1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UImage* Point2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UImage* Point3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UImage* Point4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UImage* Point5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UImage* Point6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UImage* Point7;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UImage* Point8;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = MagicControl)
	TArray<FVector2D> Positions;

	UPROPERTY(EditAnywhere, Category = MagicControl)
	TMap<FString, FSpellPoints> Spells;

protected:

	UFUNCTION(BlueprintImplementableEvent, Category = MagicControl)
	void CastedSpell(const FString& Spell);

private:
	bool bTouched;

	TArray<UImage*> Images;

	TArray<int32> Points;
};
