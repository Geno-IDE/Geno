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
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-basic-reader -o %{file.directory}/AbstractBasicReader.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-basic-writer -o %{file.directory}/AbstractBasicWriter.inc',
		}
		buildoutputs {
			'%{file.directory}/AbstractBasicReader.inc',
			'%{file.directory}/AbstractBasicWriter.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/AST/TypeProperties.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on AST/TypeProperties.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-type-reader -o %{file.directory}/AbstractTypeReader.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-type-writer -o %{file.directory}/AbstractTypeWriter.inc',
		}
		buildoutputs {
			'%{file.directory}/AbstractTypeReader.inc',
			'%{file.directory}/AbstractTypeWriter.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/Basic/Attr.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on Basic/Attr.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-impl -o %{file.directory}/../AST/AttrImpl.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-node-traverse -o %{file.directory}/../AST/AttrNodeTraverse.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-text-node-dump -o %{file.directory}/../AST/AttrTextNodeDump.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-ast-visitor -o %{file.directory}/../AST/AttrVisitor.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-classes -o %{file.directory}/../AST/Attrs.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-has-attribute-impl -o %{file.directory}/../Basic/AttrHasAttributeImpl.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-list -o %{file.directory}/../Basic/AttrList.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-subject-match-rule-list -o %{file.directory}/../Basic/AttrSubMatchRulesList.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-parser-string-switches -o %{file.directory}/../Parse/AttrParserStringSwitches.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-subject-match-rules-parser-string-switches -o %{file.directory}/../Parse/AttrSubMatchRulesParserStringSwitches.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-parsed-attr-impl -o %{file.directory}/../Sema/AttrParsedAttrImpl.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-parsed-attr-kinds -o %{file.directory}/../Sema/AttrParsedAttrKinds.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-parsed-attr-list -o %{file.directory}/../Sema/AttrParsedAttrList.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-spelling-index -o %{file.directory}/../Sema/AttrSpellingListIndex.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-template-instantiate -o %{file.directory}/../Sema/AttrTemplateInstantiate.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-pch-read -o %{file.directory}/../Serialization/AttrPCHRead.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-attr-pch-write -o %{file.directory}/../Serialization/AttrPCHWrite.inc',
		}
		buildoutputs {
			'%{file.directory}/../AST/AttrImpl.inc',
			'%{file.directory}/../AST/AttrNodeTraverse.inc',
			'%{file.directory}/../AST/AttrTextNodeDump.inc',
			'%{file.directory}/../AST/AttrVisitor.inc',
			'%{file.directory}/../AST/Attrs.inc',
			'%{file.directory}/../Basic/AttrHasAttributeImpl.inc',
			'%{file.directory}/../Basic/AttrList.inc',
			'%{file.directory}/../Basic/AttrSubMatchRulesList.inc',
			'%{file.directory}/../Parse/AttrParserStringSwitches.inc',
			'%{file.directory}/../Parse/AttrSubMatchRulesParserStringSwitches.inc',
			'%{file.directory}/../Sema/AttrParsedAttrImpl.inc',
			'%{file.directory}/../Sema/AttrParsedAttrKinds.inc',
			'%{file.directory}/../Sema/AttrParsedAttrList.inc',
			'%{file.directory}/../Sema/AttrSpellingListIndex.inc',
			'%{file.directory}/../Sema/AttrTemplateInstantiate.inc',
			'%{file.directory}/../Serialization/AttrPCHRead.inc',
			'%{file.directory}/../Serialization/AttrPCHWrite.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/AST/CommentCommands.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on AST/CommentCommands.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-comment-command-info -o %{file.directory}/CommentCommandInfo.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-comment-command-list -o %{file.directory}/CommentCommandList.inc',
		}
		buildoutputs {
			'%{file.directory}/CommentCommandInfo.inc',
			'%{file.directory}/CommentCommandList.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/AST/CommentHTMLNamedCharacterReferences.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on AST/CommentHTMLNamedCharacterReferences.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-comment-html-named-character-references -o %{file.directory}/CommentHTMLNamedCharacterReferences.inc',
		}
		buildoutputs {
			'%{file.directory}/CommentHTMLNamedCharacterReferences.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/AST/CommentHTMLTags.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on AST/CommentHTMLTags.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-comment-html-tags -o %{file.directory}/CommentHTMLTags.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-comment-html-tags-properties -o %{file.directory}/CommentHTMLTagsProperties.inc',
		}
		buildoutputs {
			'%{file.directory}/CommentHTMLTags.inc',
			'%{file.directory}/CommentHTMLTagsProperties.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/Basic/CommentNodes.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on Basic/CommentNodes.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-comment-nodes -o %{file.directory}/../AST/CommentNodes.inc',
		}
		buildoutputs {
			'%{file.directory}/../AST/CommentNodes.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/Basic/DeclNodes.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on Basic/DeclNodes.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-decl-nodes -o %{file.directory}/../AST/DeclNodes.inc',
		}
		buildoutputs {
			'%{file.directory}/../AST/DeclNodes.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/Basic/Diagnostic.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on Basic/Diagnostic.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory} --gen-clang-diag-groups -o %{file.directory}/DiagnosticGroups.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory} --gen-clang-diags-index-name -o %{file.directory}/DiagnosticIndexName.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory} --gen-clang-diags-defs --clang-component=AST -o %{file.directory}/DiagnosticASTKinds.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory} --gen-clang-diags-defs --clang-component=Analysis -o %{file.directory}/DiagnosticAnalysisKinds.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory} --gen-clang-diags-defs --clang-component=Comment -o %{file.directory}/DiagnosticCommentKinds.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory} --gen-clang-diags-defs --clang-component=Common -o %{file.directory}/DiagnosticCommonKinds.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory} --gen-clang-diags-defs --clang-component=CrossTU -o %{file.directory}/DiagnosticCrossTUKinds.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory} --gen-clang-diags-defs --clang-component=Driver -o %{file.directory}/DiagnosticDriverKinds.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory} --gen-clang-diags-defs --clang-component=Frontend -o %{file.directory}/DiagnosticFrontendKinds.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory} --gen-clang-diags-defs --clang-component=Lex -o %{file.directory}/DiagnosticLexKinds.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory} --gen-clang-diags-defs --clang-component=Parse -o %{file.directory}/DiagnosticParseKinds.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory} --gen-clang-diags-defs --clang-component=Refactoring -o %{file.directory}/DiagnosticRefactoringKinds.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory} --gen-clang-diags-defs --clang-component=Sema -o %{file.directory}/DiagnosticSemaKinds.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory} --gen-clang-diags-defs --clang-component=Serialization -o %{file.directory}/DiagnosticSerializationKinds.inc',
		}
		buildoutputs {
			'%{file.directory}/DiagnosticASTKinds.inc',
			'%{file.directory}/DiagnosticAnalysisKinds.inc',
			'%{file.directory}/DiagnosticCommentKinds.inc',
			'%{file.directory}/DiagnosticCommonKinds.inc',
			'%{file.directory}/DiagnosticCrossTUKinds.inc',
			'%{file.directory}/DiagnosticDriverKinds.inc',
			'%{file.directory}/DiagnosticFrontendKinds.inc',
			'%{file.directory}/DiagnosticGroups.inc',
			'%{file.directory}/DiagnosticIndexName.inc',
			'%{file.directory}/DiagnosticLexKinds.inc',
			'%{file.directory}/DiagnosticParseKinds.inc',
			'%{file.directory}/DiagnosticRefactoringKinds.inc',
			'%{file.directory}/DiagnosticSemaKinds.inc',
			'%{file.directory}/DiagnosticSerializationKinds.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/AST/StmtDataCollectors.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on AST/StmtDataCollectors.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-data-collectors -o %{file.directory}/StmtDataCollectors.inc',
		}
		buildoutputs {
			'%{file.directory}/StmtDataCollectors.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/Basic/StmtNodes.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on Basic/StmtNodes.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-stmt-nodes -o %{file.directory}/../AST/StmtNodes.inc',
		}
		buildoutputs {
			'%{file.directory}/../AST/StmtNodes.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/Basic/TypeNodes.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on Basic/TypeNodes.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../.. --gen-clang-type-nodes -o %{file.directory}/../AST/TypeNodes.inc',
		}
		buildoutputs {
			'%{file.directory}/../AST/TypeNodes.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/Basic/arm_cde.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on Basic/arm_cde.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../Basic --gen-arm-cde-builtin-aliases -o %{file.directory}/arm_cde_builtin_aliases.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../Basic --gen-arm-cde-builtin-codegen -o %{file.directory}/arm_cde_builtin_cg.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../Basic --gen-arm-cde-builtin-sema -o %{file.directory}/arm_cde_builtin_sema.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../Basic --gen-arm-cde-builtin-def -o %{file.directory}/arm_cde_builtins.inc',
		}
		buildoutputs {
			'%{file.directory}/arm_cde_builtin_aliases.inc',
			'%{file.directory}/arm_cde_builtin_cg.inc',
			'%{file.directory}/arm_cde_builtin_sema.inc',
			'%{file.directory}/arm_cde_builtins.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/Basic/arm_fp16.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on Basic/arm_fp16.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../Basic --gen-arm-neon-sema -o %{file.directory}/arm_fp16.inc',
		}
		buildoutputs {
			'%{file.directory}/arm_fp16.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/Basic/arm_mve.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on Basic/arm_mve.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../Basic --gen-arm-mve-builtin-aliases -o %{file.directory}/arm_mve_builtin_aliases.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../Basic --gen-arm-mve-builtin-codegen -o %{file.directory}/arm_mve_builtin_cg.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../Basic --gen-arm-mve-builtin-sema -o %{file.directory}/arm_mve_builtin_sema.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../Basic --gen-arm-mve-builtin-def -o %{file.directory}/arm_mve_builtins.inc',
		}
		buildoutputs {
			'%{file.directory}/arm_mve_builtin_aliases.inc',
			'%{file.directory}/arm_mve_builtin_cg.inc',
			'%{file.directory}/arm_mve_builtin_sema.inc',
			'%{file.directory}/arm_mve_builtins.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/Basic/arm_neon.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on Basic/arm_neon.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../Basic --gen-arm-neon-sema -o %{file.directory}/arm_neon.inc',
		}
		buildoutputs {
			'%{file.directory}/arm_neon.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/Basic/arm_sve.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on Basic/arm_sve.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../Basic --gen-arm-sve-builtin-codegen -o %{file.directory}/arm_sve_builtin_cg.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../Basic --gen-arm-sve-builtins -o %{file.directory}/arm_sve_builtins.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../Basic --gen-arm-sve-sema-rangechecks -o %{file.directory}/arm_sve_sema_rangechecks.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../Basic --gen-arm-sve-typeflags -o %{file.directory}/arm_sve_typeflags.inc',
		}
		buildoutputs {
			'%{file.directory}/arm_sve_builtin_cg.inc',
			'%{file.directory}/arm_sve_builtins.inc',
			'%{file.directory}/arm_sve_sema_rangechecks.inc',
			'%{file.directory}/arm_sve_typeflags.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/Driver/Options.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on Driver/Options.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-tablegen" %{file.relpath} -I %{file.directory}/../../../../llvm/include --gen-opt-parser-defs -o %{file.directory}/Options.inc',
		}
		buildoutputs {
			'%{file.directory}/Options.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/StaticAnalyzer/Checkers/Checkers.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on StaticAnalyzer/Checkers/Checkers.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory} --gen-clang-sa-checkers -o %{file.directory}/Checkers.inc',
		}
		buildoutputs {
			'%{file.directory}/Checkers.inc',
		}

	filter 'files:third_party/llvm-project/clang/include/clang/Tooling/Syntax/Nodes.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on Tooling/Syntax/Nodes.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../../.. --gen-clang-syntax-node-classes -o %{file.directory}/NodeClasses.inc',
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../../.. --gen-clang-syntax-node-list -o %{file.directory}/Nodes.inc',
		}
		buildoutputs {
			'%{file.directory}/NodeClasses.inc',
			'%{file.directory}/Nodes.inc',
		}

	filter 'files:third_party/llvm-project/clang/lib/AST/Interp/Opcodes.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on AST/Interp/Opcodes.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../../../include --gen-clang-opcodes -o %{file.directory}/Opcodes.inc',
		}
		buildoutputs {
			'%{file.directory}/Opcodes.inc',
		}

	filter 'files:third_party/llvm-project/clang/lib/Sema/OpenCLBuiltins.td'
		linkbuildoutputs 'Off'
		buildmessage 'TableGen on Sema/OpenCLBuiltins.td'
		buildcommands {
			'"%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}/llvm-clang-tablegen" %{file.relpath} -I %{file.directory}/../../include --gen-clang-opencl-builtins -o %{file.directory}/OpenCLBuiltins.inc',
		}
		buildoutputs {
			'%{file.directory}/OpenCLBuiltins.inc',
		}
end
