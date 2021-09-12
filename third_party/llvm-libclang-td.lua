return function()
	kind 'Utility'
	dependson {
		'llvm-tablegen',
		'llvm-clang-tablegen',
	}
	files {
		'third_party/llvm-project/clang/include/clang/AST/CommentCommands.td',
		'third_party/llvm-project/clang/include/clang/AST/CommentHTMLNamedCharacterReferences.td',
		'third_party/llvm-project/clang/include/clang/AST/CommentHTMLTags.td',
		'third_party/llvm-project/clang/include/clang/AST/PropertiesBase.td',
		'third_party/llvm-project/clang/include/clang/AST/StmtDataCollectors.td',
		'third_party/llvm-project/clang/include/clang/AST/TypeProperties.td',
		'third_party/llvm-project/clang/include/clang/Basic/Attr.td',
		'third_party/llvm-project/clang/include/clang/Basic/CommentNodes.td',
		'third_party/llvm-project/clang/include/clang/Basic/DeclNodes.td',
		'third_party/llvm-project/clang/include/clang/Basic/Diagnostic.td',
		'third_party/llvm-project/clang/include/clang/Basic/StmtNodes.td',
		'third_party/llvm-project/clang/include/clang/Basic/TypeNodes.td',
		'third_party/llvm-project/clang/include/clang/Basic/arm_cde.td',
		'third_party/llvm-project/clang/include/clang/Basic/arm_fp16.td',
		'third_party/llvm-project/clang/include/clang/Basic/arm_mve.td',
		'third_party/llvm-project/clang/include/clang/Basic/arm_neon.td',
		'third_party/llvm-project/clang/include/clang/Basic/arm_sve.td',
		'third_party/llvm-project/clang/include/clang/Driver/Options.td',
		'third_party/llvm-project/clang/include/clang/StaticAnalyzer/Checkers/Checkers.td',
		'third_party/llvm-project/clang/include/clang/Tooling/Syntax/Nodes.td',
		'third_party/llvm-project/clang/lib/AST/Interp/Opcodes.td',
		'third_party/llvm-project/clang/lib/Sema/OpenCLBuiltins.td',
	}

	filter 'files:third_party/llvm-project/clang/include/clang/AST/PropertiesBase.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on AST/PropertiesBase.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-basic-reader -o %{wks.location}/third_party/.tablegen/clang/AST/AbstractBasicReader.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-basic-writer -o %{wks.location}/third_party/.tablegen/clang/AST/AbstractBasicWriter.inc',
		}
		buildoutputs {
			'%{wks.location}/third_party/.tablegen/clang/AST/AbstractBasicReader.inc',
			'%{wks.location}/third_party/.tablegen/clang/AST/AbstractBasicWriter.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/AST/TypeProperties.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on AST/TypeProperties.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-type-reader -o %{wks.location}/third_party/.tablegen/clang/AST/AbstractTypeReader.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-type-writer -o %{wks.location}/third_party/.tablegen/clang/AST/AbstractTypeWriter.inc',
		}
		buildoutputs {
			'%{wks.location}/third_party/.tablegen/clang/AST/AbstractTypeReader.inc',
			'%{wks.location}/third_party/.tablegen/clang/AST/AbstractTypeWriter.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/Basic/Attr.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on Basic/Attr.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-impl -o %{wks.location}/third_party/.tablegen/clang/AST/AttrImpl.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-node-traverse -o %{wks.location}/third_party/.tablegen/clang/AST/AttrNodeTraverse.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-text-node-dump -o %{wks.location}/third_party/.tablegen/clang/AST/AttrTextNodeDump.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-ast-visitor -o %{wks.location}/third_party/.tablegen/clang/AST/AttrVisitor.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-classes -o %{wks.location}/third_party/.tablegen/clang/AST/Attrs.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-has-attribute-impl -o %{wks.location}/third_party/.tablegen/clang/Basic/AttrHasAttributeImpl.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-list -o %{wks.location}/third_party/.tablegen/clang/Basic/AttrList.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-subject-match-rule-list -o %{wks.location}/third_party/.tablegen/clang/Basic/AttrSubMatchRulesList.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-parser-string-switches -o %{wks.location}/third_party/.tablegen/clang/Parse/AttrParserStringSwitches.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-subject-match-rules-parser-string-switches -o %{wks.location}/third_party/.tablegen/clang/Parse/AttrSubMatchRulesParserStringSwitches.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-parsed-attr-impl -o %{wks.location}/third_party/.tablegen/clang/Sema/AttrParsedAttrImpl.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-parsed-attr-kinds -o %{wks.location}/third_party/.tablegen/clang/Sema/AttrParsedAttrKinds.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-parsed-attr-list -o %{wks.location}/third_party/.tablegen/clang/Sema/AttrParsedAttrList.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-spelling-index -o %{wks.location}/third_party/.tablegen/clang/Sema/AttrSpellingListIndex.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-template-instantiate -o %{wks.location}/third_party/.tablegen/clang/Sema/AttrTemplateInstantiate.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-pch-read -o %{wks.location}/third_party/.tablegen/clang/Serialization/AttrPCHRead.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-pch-write -o %{wks.location}/third_party/.tablegen/clang/Serialization/AttrPCHWrite.inc',
		}
		buildoutputs {
			'%{wks.location}/third_party/.tablegen/clang/AST/AttrImpl.inc',
			'%{wks.location}/third_party/.tablegen/clang/AST/AttrNodeTraverse.inc',
			'%{wks.location}/third_party/.tablegen/clang/AST/AttrTextNodeDump.inc',
			'%{wks.location}/third_party/.tablegen/clang/AST/AttrVisitor.inc',
			'%{wks.location}/third_party/.tablegen/clang/AST/Attrs.inc',
			'%{wks.location}/third_party/.tablegen/clang/Basic/AttrHasAttributeImpl.inc',
			'%{wks.location}/third_party/.tablegen/clang/Basic/AttrList.inc',
			'%{wks.location}/third_party/.tablegen/clang/Basic/AttrSubMatchRulesList.inc',
			'%{wks.location}/third_party/.tablegen/clang/Parse/AttrParserStringSwitches.inc',
			'%{wks.location}/third_party/.tablegen/clang/Parse/AttrSubMatchRulesParserStringSwitches.inc',
			'%{wks.location}/third_party/.tablegen/clang/Sema/AttrParsedAttrImpl.inc',
			'%{wks.location}/third_party/.tablegen/clang/Sema/AttrParsedAttrKinds.inc',
			'%{wks.location}/third_party/.tablegen/clang/Sema/AttrParsedAttrList.inc',
			'%{wks.location}/third_party/.tablegen/clang/Sema/AttrSpellingListIndex.inc',
			'%{wks.location}/third_party/.tablegen/clang/Sema/AttrTemplateInstantiate.inc',
			'%{wks.location}/third_party/.tablegen/clang/Serialization/AttrPCHRead.inc',
			'%{wks.location}/third_party/.tablegen/clang/Serialization/AttrPCHWrite.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/AST/CommentCommands.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on AST/CommentCommands.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-comment-command-info -o %{wks.location}/third_party/.tablegen/clang/AST/CommentCommandInfo.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-comment-command-list -o %{wks.location}/third_party/.tablegen/clang/AST/CommentCommandList.inc',
		}
		buildoutputs {
			'%{wks.location}/third_party/.tablegen/clang/AST/CommentCommandInfo.inc',
			'%{wks.location}/third_party/.tablegen/clang/AST/CommentCommandList.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/AST/CommentHTMLNamedCharacterReferences.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on AST/CommentHTMLNamedCharacterReferences.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-comment-html-named-character-references -o %{wks.location}/third_party/.tablegen/clang/AST/CommentHTMLNamedCharacterReferences.inc',
		}
		buildoutputs {
			'%{wks.location}/third_party/.tablegen/clang/AST/CommentHTMLNamedCharacterReferences.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/AST/CommentHTMLTags.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on AST/CommentHTMLTags.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-comment-html-tags -o %{wks.location}/third_party/.tablegen/clang/AST/CommentHTMLTags.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-comment-html-tags-properties -o %{wks.location}/third_party/.tablegen/clang/AST/CommentHTMLTagsProperties.inc',
		}
		buildoutputs {
			'%{wks.location}/third_party/.tablegen/clang/AST/CommentHTMLTags.inc',
			'%{wks.location}/third_party/.tablegen/clang/AST/CommentHTMLTagsProperties.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/Basic/CommentNodes.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on Basic/CommentNodes.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-comment-nodes -o %{wks.location}/third_party/.tablegen/clang/AST/CommentNodes.inc',
		}
		buildoutputs {
			'%{wks.location}/third_party/.tablegen/clang/AST/CommentNodes.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/Basic/DeclNodes.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on Basic/DeclNodes.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-decl-nodes -o %{wks.location}/third_party/.tablegen/clang/AST/DeclNodes.inc',
		}
		buildoutputs {
			'%{wks.location}/third_party/.tablegen/clang/AST/DeclNodes.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/Basic/Diagnostic.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on Basic/Diagnostic.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory} --gen-clang-diag-groups -o %{wks.location}/third_party/.tablegen/clang/Basic/DiagnosticGroups.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory} --gen-clang-diags-index-name -o %{wks.location}/third_party/.tablegen/clang/Basic/DiagnosticIndexName.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory} --gen-clang-diags-defs --clang-component=AST -o %{wks.location}/third_party/.tablegen/clang/Basic/DiagnosticASTKinds.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory} --gen-clang-diags-defs --clang-component=Analysis -o %{wks.location}/third_party/.tablegen/clang/Basic/DiagnosticAnalysisKinds.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory} --gen-clang-diags-defs --clang-component=Comment -o %{wks.location}/third_party/.tablegen/clang/Basic/DiagnosticCommentKinds.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory} --gen-clang-diags-defs --clang-component=Common -o %{wks.location}/third_party/.tablegen/clang/Basic/DiagnosticCommonKinds.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory} --gen-clang-diags-defs --clang-component=CrossTU -o %{wks.location}/third_party/.tablegen/clang/Basic/DiagnosticCrossTUKinds.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory} --gen-clang-diags-defs --clang-component=Driver -o %{wks.location}/third_party/.tablegen/clang/Basic/DiagnosticDriverKinds.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory} --gen-clang-diags-defs --clang-component=Frontend -o %{wks.location}/third_party/.tablegen/clang/Basic/DiagnosticFrontendKinds.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory} --gen-clang-diags-defs --clang-component=Lex -o %{wks.location}/third_party/.tablegen/clang/Basic/DiagnosticLexKinds.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory} --gen-clang-diags-defs --clang-component=Parse -o %{wks.location}/third_party/.tablegen/clang/Basic/DiagnosticParseKinds.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory} --gen-clang-diags-defs --clang-component=Refactoring -o %{wks.location}/third_party/.tablegen/clang/Basic/DiagnosticRefactoringKinds.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory} --gen-clang-diags-defs --clang-component=Sema -o %{wks.location}/third_party/.tablegen/clang/Basic/DiagnosticSemaKinds.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory} --gen-clang-diags-defs --clang-component=Serialization -o %{wks.location}/third_party/.tablegen/clang/Basic/DiagnosticSerializationKinds.inc',
		}
		buildoutputs {
			'%{wks.location}/third_party/.tablegen/clang/Basic/DiagnosticASTKinds.inc',
			'%{wks.location}/third_party/.tablegen/clang/Basic/DiagnosticAnalysisKinds.inc',
			'%{wks.location}/third_party/.tablegen/clang/Basic/DiagnosticCommentKinds.inc',
			'%{wks.location}/third_party/.tablegen/clang/Basic/DiagnosticCommonKinds.inc',
			'%{wks.location}/third_party/.tablegen/clang/Basic/DiagnosticCrossTUKinds.inc',
			'%{wks.location}/third_party/.tablegen/clang/Basic/DiagnosticDriverKinds.inc',
			'%{wks.location}/third_party/.tablegen/clang/Basic/DiagnosticFrontendKinds.inc',
			'%{wks.location}/third_party/.tablegen/clang/Basic/DiagnosticGroups.inc',
			'%{wks.location}/third_party/.tablegen/clang/Basic/DiagnosticIndexName.inc',
			'%{wks.location}/third_party/.tablegen/clang/Basic/DiagnosticLexKinds.inc',
			'%{wks.location}/third_party/.tablegen/clang/Basic/DiagnosticParseKinds.inc',
			'%{wks.location}/third_party/.tablegen/clang/Basic/DiagnosticRefactoringKinds.inc',
			'%{wks.location}/third_party/.tablegen/clang/Basic/DiagnosticSemaKinds.inc',
			'%{wks.location}/third_party/.tablegen/clang/Basic/DiagnosticSerializationKinds.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/AST/StmtDataCollectors.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on AST/StmtDataCollectors.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-data-collectors -o %{wks.location}/third_party/.tablegen/clang/AST/StmtDataCollectors.inc',
		}
		buildoutputs {
			'%{wks.location}/third_party/.tablegen/clang/AST/StmtDataCollectors.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/Basic/StmtNodes.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on Basic/StmtNodes.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-stmt-nodes -o %{wks.location}/third_party/.tablegen/clang/AST/StmtNodes.inc',
		}
		buildoutputs {
			'%{wks.location}/third_party/.tablegen/clang/AST/StmtNodes.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/Basic/TypeNodes.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on Basic/TypeNodes.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-type-nodes -o %{wks.location}/third_party/.tablegen/clang/AST/TypeNodes.inc',
		}
		buildoutputs {
			'%{wks.location}/third_party/.tablegen/clang/AST/TypeNodes.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/Basic/arm_cde.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on Basic/arm_cde.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../Basic --gen-arm-cde-builtin-aliases -o %{wks.location}/third_party/.tablegen/clang/Basic/arm_cde_builtin_aliases.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../Basic --gen-arm-cde-builtin-codegen -o %{wks.location}/third_party/.tablegen/clang/Basic/arm_cde_builtin_cg.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../Basic --gen-arm-cde-builtin-sema -o %{wks.location}/third_party/.tablegen/clang/Basic/arm_cde_builtin_sema.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../Basic --gen-arm-cde-builtin-def -o %{wks.location}/third_party/.tablegen/clang/Basic/arm_cde_builtins.inc',
		}
		buildoutputs {
			'%{wks.location}/third_party/.tablegen/clang/Basic/arm_cde_builtin_aliases.inc',
			'%{wks.location}/third_party/.tablegen/clang/Basic/arm_cde_builtin_cg.inc',
			'%{wks.location}/third_party/.tablegen/clang/Basic/arm_cde_builtin_sema.inc',
			'%{wks.location}/third_party/.tablegen/clang/Basic/arm_cde_builtins.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/Basic/arm_fp16.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on Basic/arm_fp16.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../Basic --gen-arm-neon-sema -o %{wks.location}/third_party/.tablegen/clang/Basic/arm_fp16.inc',
		}
		buildoutputs {
			'%{wks.location}/third_party/.tablegen/clang/Basic/arm_fp16.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/Basic/arm_mve.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on Basic/arm_mve.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../Basic --gen-arm-mve-builtin-aliases -o %{wks.location}/third_party/.tablegen/clang/Basic/arm_mve_builtin_aliases.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../Basic --gen-arm-mve-builtin-codegen -o %{wks.location}/third_party/.tablegen/clang/Basic/arm_mve_builtin_cg.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../Basic --gen-arm-mve-builtin-sema -o %{wks.location}/third_party/.tablegen/clang/Basic/arm_mve_builtin_sema.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../Basic --gen-arm-mve-builtin-def -o %{wks.location}/third_party/.tablegen/clang/Basic/arm_mve_builtins.inc',
		}
		buildoutputs {
			'%{wks.location}/third_party/.tablegen/clang/Basic/arm_mve_builtin_aliases.inc',
			'%{wks.location}/third_party/.tablegen/clang/Basic/arm_mve_builtin_cg.inc',
			'%{wks.location}/third_party/.tablegen/clang/Basic/arm_mve_builtin_sema.inc',
			'%{wks.location}/third_party/.tablegen/clang/Basic/arm_mve_builtins.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/Basic/arm_neon.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on Basic/arm_neon.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../Basic --gen-arm-neon-sema -o %{wks.location}/third_party/.tablegen/clang/Basic/arm_neon.inc',
		}
		buildoutputs {
			'%{wks.location}/third_party/.tablegen/clang/Basic/arm_neon.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/Basic/arm_sve.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on Basic/arm_sve.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../Basic --gen-arm-sve-builtin-codegen -o %{wks.location}/third_party/.tablegen/clang/Basic/arm_sve_builtin_cg.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../Basic --gen-arm-sve-builtins -o %{wks.location}/third_party/.tablegen/clang/Basic/arm_sve_builtins.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../Basic --gen-arm-sve-sema-rangechecks -o %{wks.location}/third_party/.tablegen/clang/Basic/arm_sve_sema_rangechecks.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../Basic --gen-arm-sve-typeflags -o %{wks.location}/third_party/.tablegen/clang/Basic/arm_sve_typeflags.inc',
		}
		buildoutputs {
			'%{wks.location}/third_party/.tablegen/clang/Basic/arm_sve_builtin_cg.inc',
			'%{wks.location}/third_party/.tablegen/clang/Basic/arm_sve_builtins.inc',
			'%{wks.location}/third_party/.tablegen/clang/Basic/arm_sve_sema_rangechecks.inc',
			'%{wks.location}/third_party/.tablegen/clang/Basic/arm_sve_typeflags.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/Driver/Options.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on Driver/Options.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../../../../llvm/include --gen-opt-parser-defs -o %{wks.location}/third_party/.tablegen/clang/Driver/Options.inc',
		}
		buildoutputs {
			'%{wks.location}/third_party/.tablegen/clang/Driver/Options.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/StaticAnalyzer/Checkers/Checkers.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on StaticAnalyzer/Checkers/Checkers.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory} --gen-clang-sa-checkers -o %{wks.location}/third_party/.tablegen/clang/StaticAnalyzer/Checkers/Checkers.inc',
		}
		buildoutputs {
			'%{wks.location}/third_party/.tablegen/clang/StaticAnalyzer/Checkers/Checkers.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/Tooling/Syntax/Nodes.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on Tooling/Syntax/Nodes.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../../.. --gen-clang-syntax-node-classes -o %{wks.location}/third_party/.tablegen/clang/Tooling/Syntax/NodeClasses.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../../.. --gen-clang-syntax-node-list -o %{wks.location}/third_party/.tablegen/clang/Tooling/Syntax/Nodes.inc',
		}
		buildoutputs {
			'%{wks.location}/third_party/.tablegen/clang/Tooling/Syntax/NodeClasses.inc',
			'%{wks.location}/third_party/.tablegen/clang/Tooling/Syntax/Nodes.inc',
		}

	filter 'files:third_party/llvm-project/clang/lib/AST/Interp/Opcodes.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on AST/Interp/Opcodes.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../../../include --gen-clang-opcodes -o %{wks.location}/third_party/.tablegen/clang/AST/Interp/Opcodes.inc',
		}
		buildoutputs {
			'%{wks.location}/third_party/.tablegen/clang/AST/Interp/Opcodes.inc',
		}

	filter 'files:third_party/llvm-project/clang/lib/Sema/OpenCLBuiltins.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on Sema/OpenCLBuiltins.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../../include --gen-clang-opencl-builtins -o %{wks.location}/third_party/.tablegen/clang/Sema/OpenCLBuiltins.inc',
		}
		buildoutputs {
			'%{wks.location}/third_party/.tablegen/clang/Sema/OpenCLBuiltins.inc',
		}
end
