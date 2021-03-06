// Fill out your copyright notice in the Description page of Project Settings.


#include "MyMagicWidget.h"
#include "Components/Image.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/SlateBlueprintLibrary.h"

#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::White,text)

UMyMagicWidget::UMyMagicWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	
}

void UMyMagicWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UMyMagicWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	FSpellPoints ShieldSpell;
	FSpellPoints ThrowSpell;
	FSpellPoints CircleThrowSpell;
	FSpellPoints FireBallSpell;
	FSpellPoints FlockSpell;
	ShieldSpell.Points = {
		0,
		1
	};
	ThrowSpell.Points = {
		3,
		1
	};
	CircleThrowSpell.Points = {
		1,
		2,
		4,
		6,
		7,
		5,
		3,
		0,
		1
	};
	FireBallSpell.Points = {
		3,
		4
	};
	FlockSpell.Points = {
		2,
		4
	};
	Spells.Add(TEXT("Shield"), ShieldSpell);
	Spells.Add(TEXT("Throw"), ThrowSpell);
	Spells.Add(TEXT("ForceWave"), CircleThrowSpell);
	Spells.Add(TEXT("Fireball"), FireBallSpell);
	Spells.Add(TEXT("Flock"), FlockSpell);
	Images.Emplace(Point1);
	Images.Emplace(Point2);
	Images.Emplace(Point3);
	Images.Emplace(Point4);
	Images.Emplace(Point5);
	Images.Emplace(Point6);
	Images.Emplace(Point7);
	Images.Emplace(Point8);
}

void UMyMagicWidget::DrawMagicLines(FPaintContext Context)
{
	if (bTouched)
	{
		UWidgetBlueprintLibrary::DrawLines(Context, Positions, FLinearColor::Blue, true, 5.0f);
	}
}

void UMyMagicWidget::StartLine(FVector2D TouchPosition)
{
	int32 CurrentImage = Images.IndexOfByPredicate([](const UImage* Img) {
		if (Img)
			return Img->IsHovered() == true;
		else
			return false;
	});

	if (CurrentImage == INDEX_NONE)
	{
		bTouched = false;
		return;
	}

	bTouched = true;
	FVector2D ImagePossition = GetImagePosition(CurrentImage);
	Positions.Emplace(ImagePossition);
	Positions.Emplace(ImagePossition);
	Points.Emplace(CurrentImage);
}

void UMyMagicWidget::MoveLine(FVector2D TouchPosition)
{
	if (!bTouched)
	{
		return;
	}
	int32 CurrentImage = Images.IndexOfByPredicate([](const UImage* Img) {
		return Img->IsHovered() == true;
		});

	if (CurrentImage == INDEX_NONE)
	{
		Positions.Last().Set(TouchPosition.X, TouchPosition.Y);
		return;
	}
	if (CheckPoints(CurrentImage)) {
		FVector2D ImagePossition = GetImagePosition(CurrentImage);
		Positions.Last().Set(ImagePossition.X, ImagePossition.Y);
		Positions.Emplace(ImagePossition);
		Points.Emplace(CurrentImage);
		FString Spell = CheckForSpell();
		return;
	}
}

void UMyMagicWidget::EndLine()
{
	bTouched = false;
	Positions.Empty();
	Points.Empty();
}

FVector2D UMyMagicWidget::GetImagePosition(int32 ImageIndex)
{
	FVector2D PixelPos;
	FVector2D ViewportPos;
	FVector2D ImageSize = Images[ImageIndex]->GetCachedGeometry().GetLocalSize();
	USlateBlueprintLibrary::LocalToViewport(GetWorld(), Images[ImageIndex]->GetCachedGeometry(), FVector2D(0, 0), PixelPos, ViewportPos);
	return FVector2D(ViewportPos.X + ImageSize.X / 2, ViewportPos.Y + ImageSize.Y / 2);
}

bool UMyMagicWidget::CheckPoints(int32 ImageIndex)
{
	int32 Num = Points.Num();
	int32 pointA = ImageIndex;
	int32 pointB = Points.Last();
	if (pointA == pointB)
	{
		return false;
	}
	if (Num < 2) {
		return true;
	}
	for (int32 i = 0; i < Num - 1; i++)
	{
		if (pointA == Points[i] && pointB == Points[i + 1])
		{
			return false;
		}
		if (pointB == Points[i] && pointA == Points[i + 1])
		{
			return false;
		}
	}
	return true;
}

FString UMyMagicWidget::CheckForSpell()
{
	for (auto& Elem : Spells)
	{
		if (Elem.Value.Points == Points)
		{
			CastedSpell(Elem.Key);
			print(FString::Printf(TEXT("Spell is %s"), &Elem.Key));
		}
	}
	return "SPELL_NONE";
}
