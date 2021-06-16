#include "TypeHandling.hpp"



myJSInstance::myJSInstance() = default;

bool myJSInstance::RegisterFuncts(RE::BSScript::Internal::VirtualMachine* a_registry)
{
	a_registry->RegisterFunction("CallGlobalFunction", "CallGlobalFuncts", CallGlobalFunction);
	a_registry->RegisterFunction("CallInstanceFunction", "CallGlobalFuncts", CallInstanceFunction);
	return true;
}

RE::BSScript::ObjectTypeInfo::GlobalFuncInfo* myJSInstance::GetGlobalFunction(RE::BSScript::Internal::VirtualMachine* impvm, std::vector<std::string> classfunctSplitParts, std::uint32_t numArgs)
{
	for (const auto& object_type : impvm->objectTypeMap) {
		if (strcmp(object_type.first.c_str(), classfunctSplitParts.at(0).c_str()) == 0) {
			auto objectInfo = object_type.second;
			for (std::uint32_t index = 0; index < objectInfo->GetNumGlobalFuncs(); ++index) {
				const auto globalFunct = objectInfo->GetGlobalFuncIter() + index;
				if (strcmp(globalFunct->func->GetName().c_str(), classfunctSplitParts.at(1).c_str()) == 0) {
					if (globalFunct->func->GetParamCount() == numArgs) {
						return globalFunct;
					}
				}
			}
		}
	}
	return nullptr;
}
RE::BSScript::ObjectTypeInfo::MemberFuncInfo* myJSInstance::GetMemberFunction(RE::BSScript::Internal::VirtualMachine* impvm, std::vector<std::string> classfunctSplitParts, std::uint32_t numArgs)
{
	for (const auto& object_type : impvm->objectTypeMap) {
		if (strcmp(object_type.first.c_str(), classfunctSplitParts.at(0).c_str()) == 0) {
			auto objectInfo = object_type.second;
			for (std::uint32_t index = 0; index < objectInfo->GetNumMemberFuncs(); ++index) {
				const auto globalFunct = objectInfo->GetMemberFuncIter() + index;
				if (strcmp(globalFunct->func->GetName().c_str(), classfunctSplitParts.at(1).c_str()) == 0) {
					if (globalFunct->func->GetParamCount() == numArgs) {
						return globalFunct;
					}
				}
			}
		}
	}
	return nullptr;
}
std::vector<RE::BSScript::TypeInfo> myJSInstance::GetFunctArgs(RE::BSScript::ObjectTypeInfo::GlobalFuncInfo* globalFunct)
{
	auto innerFunct = globalFunct->func;
	return getFunctArgsBody(innerFunct);
}
std::vector<RE::BSScript::TypeInfo> myJSInstance::GetFunctArgs(RE::BSScript::ObjectTypeInfo::MemberFuncInfo* globalFunct)
{
	auto innerFunct = globalFunct->func;
	return getFunctArgsBody(innerFunct);
}
std::vector<RE::BSScript::TypeInfo> myJSInstance::getFunctArgsBody(const RE::BSTSmartPointer<RE::BSScript::IFunction>& innerFunct)
{
	std::vector<RE::BSScript::TypeInfo> ParamData;
	for (uint32_t paramIndex = 0; paramIndex < innerFunct->GetParamCount(); ++paramIndex) {
		RE::BSFixedString paramName;
		RE::BSScript::TypeInfo paramType;
		innerFunct->GetParam(paramIndex, paramName, paramType);
		ParamData.push_back(paramType);
	}
	return ParamData;
}
RE::BSScript::IFunctionArguments* myJSInstance::ConvertArgs(RE::BSScript::ObjectTypeInfo::MemberFuncInfo* globalFunct, std::vector<std::string> args)
{
	auto argvals = GetFunctArgs(globalFunct);
	return getArgumentsBody(args, argvals);
}
RE::BSScript::IFunctionArguments* myJSInstance::ConvertArgs(RE::BSScript::ObjectTypeInfo::GlobalFuncInfo* globalFunct, std::vector<std::string> args)
{
	auto argvals = GetFunctArgs(globalFunct);
	return getArgumentsBody(args, argvals);
}
RE::BSScript::IFunctionArguments* myJSInstance::getArgumentsBody(std::vector<std::string>& args, std::vector<RE::BSScript::TypeInfo>& argvals)
{
	RE::BSScript::TypeInfo typeValOne;
	RE::BSScript::TypeInfo typeValtwo;
	RE::BSScript::TypeInfo typeValthree;
	std::string valStringOne;
	std::string valStringTwo;
	std::string valStringThree;
	RE::BSScript::IFunctionArguments* value1 = RE::MakeFunctionArguments();
	if (!argvals.empty()) {
		typeValOne = argvals.at(0);
		valStringOne = args.at(0);
	}
	if (argvals.size() >= 2) {
		typeValtwo = argvals.at(1);
		valStringTwo = args.at(1);
	}
	if (argvals.size() == 3) {
		typeValthree = argvals.at(2);
		valStringThree = args.at(2);
	}
	if (argvals.size() == 1) {
        TypeHandling::HandleSingleValue(typeValOne, valStringOne, value1);
	}
	if (argvals.size() == 2) {
        TypeHandling::HandleTwoValues(typeValOne, typeValtwo, valStringOne, valStringTwo, value1);
	}
	if (argvals.size() == 3) {
        TypeHandling::HandleThreeValues(typeValOne, typeValtwo, typeValthree, valStringOne, valStringTwo, valStringThree, value1);
	}
	return value1;
}


void myJSInstance::CallGlobalFunction([[maybe_unused]] RE::StaticFunctionTag* aaa, RE::BSFixedString classfunct, RE::BSFixedString arglist)
{
	std::vector<std::string> classfunctSplitParts = RemoveQuotesAndSplit(classfunct, '.');
	std::vector<std::string> functionArgs = RemoveQuotesAndSplit(arglist, ',');
	auto impvm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
	const auto globalFunct = GetGlobalFunction(impvm, classfunctSplitParts, static_cast<std::uint32_t>(functionArgs.size()));
	if (globalFunct == nullptr)
		return;
	const auto functargs = ConvertArgs(globalFunct, functionArgs);
	RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> aaaclass;
	impvm->DispatchStaticCall(globalFunct->func->GetObjectTypeName(), globalFunct->func->GetName(), functargs, aaaclass);
}
void myJSInstance::CallInstanceFunction([[maybe_unused]] RE::StaticFunctionTag* aaa, RE::BSFixedString classfunct, RE::BSFixedString arglist)
{
	std::vector<std::string> classfunctSplitParts = RemoveQuotesAndSplit(classfunct, '.');
	std::vector<std::string> functionArgs = RemoveQuotesAndSplit(arglist, ',');

	auto impvm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
	//grab form from vector
	const auto ObjectVmHandle = StringToVmHandle<RE::TESForm>(impvm, functionArgs.front());
	functionArgs.erase(functionArgs.begin());
	const auto globalFunct = GetMemberFunction(impvm, classfunctSplitParts, static_cast<std::uint32_t>(functionArgs.size()));
	if (globalFunct == nullptr)
		return;
	const auto functargs = ConvertArgs(globalFunct, functionArgs);
	RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> aaaclass;
	impvm->DispatchMethodCall(ObjectVmHandle, globalFunct->func->GetObjectTypeName(), globalFunct->func->GetName(), functargs, aaaclass);
	//impvm->DispatchStaticCall(globalFunct->func->GetObjectTypeName(), globalFunct->func->GetName(), functargs, aaaclass);
}
