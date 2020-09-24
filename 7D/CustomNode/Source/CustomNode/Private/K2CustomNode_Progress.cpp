
#include "Classes/K2CustomNode_Progress.h"



#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintEditorUtils.h"
#include "BlueprintNodeSpawner.h"
#include "EdGraphSchema_K2.h"
#include "EdGraphUtilities.h"
#include "EditorCategoryUtils.h"
#include "GraphEditorActions.h"
#include "K2Node_AssignmentStatement.h"
#include "K2Node_CallFunction.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_TemporaryVariable.h"
#include "KismetCompiledFunctionContext.h"
#include "KismetCompiler.h"
#include "KismetCompilerMisc.h"
#include "ScopedTransaction.h"
#include "ToolMenu.h"
#include "UIAction.h"
#include "Kismet/KismetMathLibrary.h"

#define LOCTEXT_NAMESPACE "CustomNode"


//    /////////////////////////////////////////////////////////////////////////////
//    FKCHandler_ExecutionSequence
//    /////////////////////////////////////////////////////////////////////////////


/*class FKCHandler_Progress : public FNodeHandlingFunctor
{
protected:
	TMap<UEdGraphNode*, FBPTerminal*> TotalInputCountMap;
	TMap<UEdGraphNode*, FBPTerminal*> CurrentInputCountMap;
	TMap<UEdGraphNode*, FBPTerminal*> BoolTermMap;
	
public:
	FKCHandler_Progress(FKismetCompilerContext& InCompilerContext)
		: FNodeHandlingFunctor(InCompilerContext)
	{
	}
	
	virtual void RegisterNets(FKismetFunctionContext& Context, UEdGraphNode* Node) override
	{
		FNodeHandlingFunctor::RegisterNets(Context, Node);

		{
			FBPTerminal* TotalTerm = Context.CreateLocalTerminal();
			TotalTerm->Type.PinCategory = UEdGraphSchema_K2::PC_Int;
			TotalTerm->bIsLiteral = true;
			TotalTerm->Source = Node;
			const int32 Flag_Total = ( 1 << Node->Pins.Num() ) - 1;
			TotalTerm->Name = FString::FromInt( Flag_Total );
			TotalInputCountMap.Add(Node, TotalTerm);
		}

		{
			FBPTerminal* CurrentTerm = Context.CreateLocalTerminal();
			CurrentTerm->Type.PinCategory = UEdGraphSchema_K2::PC_Int;
			CurrentTerm->bIsLiteral = true;
			CurrentTerm->Source = Node;
			CurrentTerm->Name = FString::FromInt( 0 );
			TotalInputCountMap.Add(Node, CurrentTerm);
		}
		
		{
			FBPTerminal* BoolTerm = Context.CreateLocalTerminal();
			BoolTerm->Type.PinCategory = UEdGraphSchema_K2::PC_Boolean;
			BoolTerm->Source = Node;
			BoolTerm->Name = Context.NetNameMap->MakeValidName(Node) + TEXT("_CmpResult");
			BoolTermMap.Add(Node, BoolTerm);
		}
	}


	bool ValidatePins(FKismetFunctionContext& Context, UEdGraphNode* Node) const
	{
		// Make sure that the input pin is connected and valid for this block
		FEdGraphPinType ExpectedPinType;
		ExpectedPinType.PinCategory = UEdGraphSchema_K2::PC_Exec;

		for(const auto& p : Node->Pins)
		{
			if(!Context.ValidatePinType(p, ExpectedPinType))
			{
				CompilerContext.MessageLog.Error(*LOCTEXT("NoValidExecutionPinForExecSeq_Error", "@@ must have a valid execution pin @@").ToString(), Node, p);
				return false;
			}
			if(p->Direction == EGPD_Input && p->LinkedTo.Num() == 0)
			{
				CompilerContext.MessageLog.Warning(*LOCTEXT("NodeNeverExecuted_Warning", "@@ will never be executed").ToString(), Node);
				return false;
			}
		}

		return true;
	}

	virtual void Compile(FKismetFunctionContext& Context, UEdGraphNode* Node) override
	{
		if(!ValidatePins(Context, Node))
		{
			return;
		}

		// Find the valid, connected output pins, and add them to the processing list
		TArray<UEdGraphPin*> InputPins;
		for (UEdGraphPin* CurrentPin : Node->Pins)
		{
			if ((CurrentPin->Direction == EGPD_Input) && (CurrentPin->LinkedTo.Num() > 0) && (CurrentPin->PinName.ToString().StartsWith(UEdGraphSchema_K2::PC_Exec.ToString())))
			{
				InputPins.Add(CurrentPin);
			}
		}
		
		UK2CustomNode_Progress* MyNode = CastChecked<UK2CustomNode_Progress>(Node);
		UEdGraphPin* InputPin = Context.FindRequiredPinByName(Node, "Execute1", EGPD_Input);

		UEdGraphPin* PinToTry = FEdGraphUtilities::GetNetFromPin(InputPin);
		FBPTerminal** pInputTerm = Context.NetMap.Find(PinToTry);

		FBPTerminal* InputTerm = *pInputTerm;

		//@TODO: Sort the pins by the number appended to the pin!
		/*
		// Process the pins, if there are any valid entries
		if (OutputPins.Num() > 0)
		{
			if (Context.IsDebuggingOrInstrumentationRequired() && (OutputPins.Num() > 1))
			{
				const FString NodeComment = Node->NodeComment.IsEmpty() ? Node->GetName() : Node->NodeComment;

				// Assuming sequence X goes to A, B, C, we want to emit:
				//   X: push X1
				//      goto A
				//  X1: debug site
				//      push X2
				//      goto B
				//  X2: debug site
				//      goto C

				// A push statement we need to patch up on the next pass (e.g., push X1 before we know where X1 is)
				FBlueprintCompiledStatement* LastPushStatement = NULL;

				for (int32 i = 0; i < OutputPins.Num(); ++i)
				{
					// Emit the debug site and patch up the previous jump if we're on subsequent steps
					const bool bNotFirstIndex = i > 0;
					if (bNotFirstIndex)
					{
						// Emit a debug site
						FBlueprintCompiledStatement& DebugSiteAndJumpTarget = Context.AppendStatementForNode(Node);
						DebugSiteAndJumpTarget.Type = Context.GetBreakpointType();
						DebugSiteAndJumpTarget.Comment = NodeComment;
						DebugSiteAndJumpTarget.bIsJumpTarget = true;

						// Patch up the previous push jump target
						check(LastPushStatement);
						LastPushStatement->TargetLabel = &DebugSiteAndJumpTarget;
					}

					// Emit a push to get to the next step in the sequence, unless we're the last one or this is an instrumented build
					const bool bNotLastIndex = ((i + 1) < OutputPins.Num());
					if (bNotLastIndex)
					{
						FBlueprintCompiledStatement& PushExecutionState = Context.AppendStatementForNode(Node);
						PushExecutionState.Type = KCST_PushState;
						LastPushStatement = &PushExecutionState;
					}

					// Emit the goto to the actual state
					FBlueprintCompiledStatement& GotoSequenceLinkedState = Context.AppendStatementForNode(Node);
					GotoSequenceLinkedState.Type = KCST_UnconditionalGoto;
					Context.GotoFixupRequestMap.Add(&GotoSequenceLinkedState, OutputPins[i]);
				}

				check(LastPushStatement);
			}
			else
			{
				// Directly emit pushes to execute the remaining branches
				for (int32 i = OutputPins.Num() - 1; i > 0; i--)
				{
					FBlueprintCompiledStatement& PushExecutionState = Context.AppendStatementForNode(Node);
					PushExecutionState.Type = KCST_PushState;
					Context.GotoFixupRequestMap.Add(&PushExecutionState, OutputPins[i]);
				}

				// Immediately jump to the first pin
				UEdGraphNode* NextNode = OutputPins[0]->LinkedTo[0]->GetOwningNode();
				FBlueprintCompiledStatement& NextExecutionState = Context.AppendStatementForNode(Node);
				NextExecutionState.Type = KCST_UnconditionalGoto;
				Context.GotoFixupRequestMap.Add(&NextExecutionState, OutputPins[0]);
			}
		}
		else
		{
			FBlueprintCompiledStatement& NextExecutionState = Context.AppendStatementForNode(Node);
			NextExecutionState.Type = KCST_EndOfThread;
		}
		*
	}
};
		*/


//    /////////////////////////////////////////////////////////////////////////////
//    UK2CustomNode_Progress
//    /////////////////////////////////////////////////////////////////////////////

class UK2Node_IfThenElse;
class UK2Node_AssignmentStatement;

UK2CustomNode_Progress::UK2CustomNode_Progress(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UK2CustomNode_Progress::AllocateDefaultPins()
{
    Super::AllocateDefaultPins();

    CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);

    // Add two default pins
    CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, GetPinNameGivenIndex(0));
    CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, GetPinNameGivenIndex(1));
}

FText UK2CustomNode_Progress::GetTooltipText() const
{
	return LOCTEXT("ExecutePinWhenAllReady_Tooltip", "Executes when all input get ready");
}

FLinearColor UK2CustomNode_Progress::GetNodeTitleColor() const
{
    return FLinearColor::Red;
}

FText UK2CustomNode_Progress::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("Progress", "Progress");
}

void UK2CustomNode_Progress::ReallocatePinsDuringReconstruction(::TArray<UEdGraphPin*>& OldPins)
{
    CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
    
    // Create a new pin for each old execution output pin, and coerce the names to match on both sides
    int32 ExecOutPinCount = 0;
    for (int32 i = 0; i < OldPins.Num(); ++i)
    {
        UEdGraphPin* TestPin = OldPins[i];
        if (UEdGraphSchema_K2::IsExecPin(*TestPin))
        {
            if(TestPin->Direction == EGPD_Input)
            {
                const FName NewPinName(GetPinNameGivenIndex(ExecOutPinCount));
                ExecOutPinCount++;

                // Make sure the old pin and new pin names match
                TestPin->PinName = NewPinName;

                // Create the new output pin to match
                CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, NewPinName);
            }
        }
    }
}

/*FNodeHandlingFunctor* UK2CustomNode_Progress::CreateNodeHandler(FKismetCompilerContext& CompilerContext) const
{
    return new FKCHandler_Progress(CompilerContext);
}*/

void UK2CustomNode_Progress::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
    Super::GetNodeContextMenuActions(Menu, Context);

    if (!Context->bIsDebugging)
    {
        static FName CommutativeAssociativeBinaryOperatorNodeName = FName("CommutativeAssociativeBinaryOperatorNode");
        FText CommutativeAssociativeBinaryOperatorStr = LOCTEXT("CommutativeAssociativeBinaryOperatorNode", "Operator Node");
        if (Context->Pin != NULL)
        {
            if(CanRemovePin(Context->Pin))
            {
                FToolMenuSection& Section = Menu->AddSection(CommutativeAssociativeBinaryOperatorNodeName, CommutativeAssociativeBinaryOperatorStr);
                Section.AddMenuEntry(
                    "RemovePin",
                    LOCTEXT("RemovePin", "Remove pin"),
                    LOCTEXT("RemovePinTooltip", "Remove this input pin"),
                    FSlateIcon(),
                    FUIAction(
                        FExecuteAction::CreateUObject(const_cast<UK2CustomNode_Progress*>(this), &UK2CustomNode_Progress::RemovePinFromProgressNode, const_cast<UEdGraphPin*>(Context->Pin))
                    )
                );
            }
        }
    }
}

void UK2CustomNode_Progress::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    // actions get registered under specific object-keys; the idea is that 
    // actions might have to be updated (or deleted) if their object-key is  
    // mutated (or removed)... here we use the node's class (so if the node 
    // type disappears, then the action should go with it)
    UClass* ActionKey = GetClass();
    // to keep from needlessly instantiating a UBlueprintNodeSpawner, first   
    // check to make sure that the registrar is looking for actions of this type
    // (could be regenerating actions for a specific asset, and therefore the 
    // registrar would only accept actions corresponding to that asset)
    if (ActionRegistrar.IsOpenForRegistration(ActionKey))
    {
        UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
        check(NodeSpawner != nullptr);

        ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
    }
}

FText UK2CustomNode_Progress::GetMenuCategory() const
{
	return FEditorCategoryUtils::GetCommonCategory(FCommonEditorCategory::FlowControl);
}

void UK2CustomNode_Progress::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();
	
	/////////////////////////////
	// Temporary Variable node
	/////////////////////////////

	// Create the current flag node
	UK2Node_TemporaryVariable* FlagCurrentNode = SourceGraph->CreateIntermediateNode<UK2Node_TemporaryVariable>();
	FlagCurrentNode->VariableType.PinCategory = UEdGraphSchema_K2::PC_Int;
	FlagCurrentNode->AllocateDefaultPins();
	CompilerContext.MessageLog.NotifyIntermediateObjectCreation(FlagCurrentNode, this);

	//Equal node
	UK2Node_CallFunction* EqualNode = SourceGraph->CreateIntermediateNode<UK2Node_CallFunction>();
	EqualNode->SetFromFunction( FindField<UFunction>(UKismetMathLibrary::StaticClass(), "EqualEqual_IntInt") );
	EqualNode->AllocateDefaultPins();
	for(auto func_pin : EqualNode->Pins)
	{
		if(func_pin->PinName == "A")
		{
			func_pin->MakeLinkTo(FlagCurrentNode->GetVariablePin());
		}
		else if(func_pin->PinName == "B")
		{
			func_pin->DefaultValue = FString::FromInt(  ( 1 << (Pins.Num() - 1) ) - 1 );
		}
	}
	
	// BranchNode
	UK2Node_IfThenElse* BranchNode = SourceGraph->CreateIntermediateNode<UK2Node_IfThenElse>();
	BranchNode->AllocateDefaultPins();
	CompilerContext.MessageLog.NotifyIntermediateObjectCreation(BranchNode, this);
	BranchNode->GetConditionPin()->MakeLinkTo(EqualNode->GetReturnValuePin());


	UEdGraphPin* OutputPin = nullptr;
	
	int32 ActualIndex = 0;
	for (int32 index = 0; index < Pins.Num(); ++index)
	{
		UEdGraphPin* p = Pins[index];
		if(p->Direction == EGPD_Output)
		{
			OutputPin = p;
			continue;
		}

		UK2Node_CallFunction* BitOrNode = SourceGraph->CreateIntermediateNode<UK2Node_CallFunction>();
		BitOrNode->SetFromFunction( FindField<UFunction>(UKismetMathLibrary::StaticClass(), "Or_IntInt") );
		BitOrNode->AllocateDefaultPins();
		for(auto func_pin : BitOrNode->Pins)
		{
			if(func_pin->PinName == "A")
			{
				func_pin->MakeLinkTo(FlagCurrentNode->GetVariablePin());
			}
			else if(func_pin->PinName == "B")
			{
				func_pin->DefaultValue = FString::FromInt( 1 << ActualIndex );
			}
		}
		
		// Assignment current Node
		UK2Node_AssignmentStatement* AssignmentCurrentNode = SourceGraph->CreateIntermediateNode<UK2Node_AssignmentStatement>();
		AssignmentCurrentNode->AllocateDefaultPins();
		CompilerContext.MessageLog.NotifyIntermediateObjectCreation(AssignmentCurrentNode, this);
		UEdGraphPin* FlagTotalVarPin = FlagCurrentNode->GetVariablePin();
		AssignmentCurrentNode->GetVariablePin()->PinType = FlagTotalVarPin->PinType;
		AssignmentCurrentNode->GetVariablePin()->MakeLinkTo(FlagTotalVarPin);
		AssignmentCurrentNode->GetValuePin()->PinType = BitOrNode->GetReturnValuePin()->PinType;
		AssignmentCurrentNode->GetValuePin()->MakeLinkTo(BitOrNode->GetReturnValuePin());
		AssignmentCurrentNode->GetThenPin()->MakeLinkTo(BranchNode->GetExecPin());

		CompilerContext.MovePinLinksToIntermediate(*p, *AssignmentCurrentNode->GetExecPin());
		
		++ActualIndex;
	}
	
	CompilerContext.MovePinLinksToIntermediate(*OutputPin, *BranchNode->GetThenPin());
	
	// Break all links to the Select node so it goes away for at scheduling time
	BreakAllNodeLinks();
}

void UK2CustomNode_Progress::AddInputPin()
{
    Modify();
    CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, GetUniquePinName());
}

bool UK2CustomNode_Progress::CanAddPin() const
{
	//for safety
    return Pins.Num() <= 30;
}

FName UK2CustomNode_Progress::GetUniquePinName()
{
    FName NewPinName;
    int32 i = 0;
    while (true)
    {
        NewPinName = GetPinNameGivenIndex(i++);
        if (!FindPin(NewPinName))
        {
            break;
        }
    }

    return NewPinName;
}

void UK2CustomNode_Progress::RemovePinFromProgressNode(UEdGraphPin* TargetPin)
{
	if (CanRemovePin(TargetPin))
	{
		FScopedTransaction Transaction(LOCTEXT("RemovePinTx", "RemovePin"));
		Modify();

		int32 PinRemovalIndex = INDEX_NONE;
		if (Pins.Find(TargetPin, /*out*/ PinRemovalIndex))
		{
			Pins.RemoveAt(PinRemovalIndex);
			TargetPin->MarkPendingKill();
		    
			FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(GetBlueprint());
		}

		// Renumber the pins so the numbering is compact
		int32 ThenIndex = 0;
		for (int32 i = 0; i < Pins.Num(); ++i)
		{
			UEdGraphPin* PotentialPin = Pins[i];
			if (UEdGraphSchema_K2::IsExecPin(*PotentialPin) && (PotentialPin->Direction == EGPD_Input))
			{
				PotentialPin->PinName = GetPinNameGivenIndex(ThenIndex);
				++ThenIndex;
			}
		}
	}
}

FName UK2CustomNode_Progress::GetPinNameGivenIndex(int32 Index) const
{
	return *FString::Printf(TEXT("%s_%d"), *UEdGraphSchema_K2::PN_Execute.ToString(), Index);
}

bool UK2CustomNode_Progress::CanRemovePin(const UEdGraphPin* Pin) const
{
    return (
        Pin &&
        Pins.Num() > 3 &&
        (INDEX_NONE != Pins.IndexOfByKey(Pin)) &&
        (EEdGraphPinDirection::EGPD_Input == Pin->Direction)
    );
}

int32 UK2CustomNode_Progress::LeftShiftPinMask(int32 PinIndex)
{
	return ( (1 << PinIndex) );
}

#undef LOCTEXT_NAMESPACE
