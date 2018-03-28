add_llvm_loadable_module( CSE231-DFA
  231DFA.h
  ReachingDefinitionAnalysis.cpp
  LivenessAnalysis.cpp
  MayPointToAnalysis.cpp

  PLUGIN_TOOL
  opt
  )
