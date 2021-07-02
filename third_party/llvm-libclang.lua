
return function()
	local function clang_diag_gen( component )
		return 'echo Clang TableGen on Basic/Diagnostic'..component..'Kinds.inc && "%{path.join(wks.location,"bin",cfg.platform,cfg.buildcfg,"llvm-clang-tablegen")}" "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic/Diagnostic.td")}" --gen-clang-diags-defs --clang-component='..component..' -o "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic/Diagnostic'..component..'Kinds.inc")}" -I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"'
	end

	dependson {
		'llvm-tablegen',
		'llvm-clang-tablegen',
	}
	includedirs {
		'third_party/llvm-project/clang/lib/Basic',
		'third_party/llvm-project/clang/lib/Driver',
	}
	sysincludedirs {
		'third_party/.config',
		'third_party/llvm-project/clang/include',
		'third_party/llvm-project/llvm/include',
	}
	prebuildcommands {
		tablegen( '../../../clang/include/clang/Driver/Options.inc', '../../../clang/include/clang/Driver/Options.td', '--gen-opt-parser-defs -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'Frontend/OpenMP/OMP.h.inc',                       'Frontend/OpenMP/OMP.td',                         '--gen-directive-decl -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'Frontend/OpenMP/OMP.inc',                         'Frontend/OpenMP/OMP.td',                         '--gen-directive-gen -I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),

		clang_tablegen( '../../lib/AST/Interp/Opcodes.inc',                '../../lib/AST/Interp/Opcodes.td',            '--gen-clang-opcodes -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( '../../lib/Sema/OpenCLBuiltins.inc',               '../../lib/Sema/OpenCLBuiltins.td',           '--gen-clang-opencl-builtins -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/AbstractBasicReader.inc',                     'AST/PropertiesBase.td',                      '--gen-clang-basic-reader -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/AbstractBasicWriter.inc',                     'AST/PropertiesBase.td',                      '--gen-clang-basic-writer -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/AbstractTypeReader.inc',                      'AST/TypeProperties.td',                      '--gen-clang-type-reader -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/AbstractTypeWriter.inc',                      'AST/TypeProperties.td',                      '--gen-clang-type-writer -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/AttrImpl.inc',                                'Basic/Attr.td',                              '--gen-clang-attr-impl -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/AttrNodeTraverse.inc',                        'Basic/Attr.td',                              '--gen-clang-attr-node-traverse -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/AttrTextNodeDump.inc',                        'Basic/Attr.td',                              '--gen-clang-attr-text-node-dump -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/AttrVisitor.inc',                             'Basic/Attr.td',                              '--gen-clang-attr-ast-visitor -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/Attrs.inc',                                   'Basic/Attr.td',                              '--gen-clang-attr-classes -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/CommentCommandInfo.inc',                      'AST/CommentCommands.td',                     '--gen-clang-comment-command-info -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/CommentCommandList.inc',                      'AST/CommentCommands.td',                     '--gen-clang-comment-command-list -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/CommentHTMLNamedCharacterReferences.inc',     'AST/CommentHTMLNamedCharacterReferences.td', '--gen-clang-comment-html-named-character-references -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/CommentHTMLTags.inc',                         'AST/CommentHTMLTags.td',                     '--gen-clang-comment-html-tags -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/CommentHTMLTagsProperties.inc',               'AST/CommentHTMLTags.td',                     '--gen-clang-comment-html-tags-properties -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/CommentNodes.inc',                            'Basic/CommentNodes.td',                      '--gen-clang-comment-nodes -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/DeclNodes.inc',                               'Basic/DeclNodes.td',                         '--gen-clang-decl-nodes -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/StmtDataCollectors.inc',                      'AST/StmtDataCollectors.td',                  '--gen-clang-data-collectors -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/StmtNodes.inc',                               'Basic/StmtNodes.td',                         '--gen-clang-stmt-nodes -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/TypeNodes.inc',                               'Basic/TypeNodes.td',                         '--gen-clang-type-nodes -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'Basic/AttrHasAttributeImpl.inc',                  'Basic/Attr.td',                              '--gen-clang-attr-has-attribute-impl -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'Basic/AttrList.inc',                              'Basic/Attr.td',                              '--gen-clang-attr-list -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'Basic/AttrSubMatchRulesList.inc',                 'Basic/Attr.td',                              '--gen-clang-attr-subject-match-rule-list -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'Basic/DiagnosticGroups.inc',                      'Basic/Diagnostic.td',                        '--gen-clang-diag-groups -I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"' ),
		clang_tablegen( 'Basic/DiagnosticIndexName.inc',                   'Basic/Diagnostic.td',                        '--gen-clang-diags-index-name -I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"' ),
		clang_tablegen( 'Basic/arm_cde_builtin_aliases.inc',               'Basic/arm_cde.td',                           '--gen-arm-cde-builtin-aliases -I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"' ),
		clang_tablegen( 'Basic/arm_cde_builtin_cg.inc',                    'Basic/arm_cde.td',                           '--gen-arm-cde-builtin-codegen -I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"' ),
		clang_tablegen( 'Basic/arm_cde_builtin_sema.inc',                  'Basic/arm_cde.td',                           '--gen-arm-cde-builtin-sema -I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"' ),
		clang_tablegen( 'Basic/arm_cde_builtins.inc',                      'Basic/arm_cde.td',                           '--gen-arm-cde-builtin-def -I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"' ),
		clang_tablegen( 'Basic/arm_fp16.inc',                              'Basic/arm_fp16.td',                          '--gen-arm-neon-sema -I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"' ),
		clang_tablegen( 'Basic/arm_mve_builtin_aliases.inc',               'Basic/arm_mve.td',                           '--gen-arm-mve-builtin-aliases -I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"' ),
		clang_tablegen( 'Basic/arm_mve_builtin_cg.inc',                    'Basic/arm_mve.td',                           '--gen-arm-mve-builtin-codegen -I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"' ),
		clang_tablegen( 'Basic/arm_mve_builtin_sema.inc',                  'Basic/arm_mve.td',                           '--gen-arm-mve-builtin-sema -I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"' ),
		clang_tablegen( 'Basic/arm_mve_builtins.inc',                      'Basic/arm_mve.td',                           '--gen-arm-mve-builtin-def -I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"' ),
		clang_tablegen( 'Basic/arm_neon.inc',                              'Basic/arm_neon.td',                          '--gen-arm-neon-sema -I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"' ),
		clang_tablegen( 'Basic/arm_sve_builtin_cg.inc',                    'Basic/arm_sve.td',                           '--gen-arm-sve-builtin-codegen -I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"' ),
		clang_tablegen( 'Basic/arm_sve_builtins.inc',                      'Basic/arm_sve.td',                           '--gen-arm-sve-builtins -I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"' ),
		clang_tablegen( 'Basic/arm_sve_sema_rangechecks.inc',              'Basic/arm_sve.td',                           '--gen-arm-sve-sema-rangechecks -I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"' ),
		clang_tablegen( 'Basic/arm_sve_typeflags.inc',                     'Basic/arm_sve.td',                           '--gen-arm-sve-typeflags -I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"' ),
		clang_tablegen( 'Parse/AttrParserStringSwitches.inc',              'Basic/Attr.td',                              '--gen-clang-attr-parser-string-switches -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'Parse/AttrSubMatchRulesParserStringSwitches.inc', 'Basic/Attr.td',                              '--gen-clang-attr-subject-match-rules-parser-string-switches -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'Sema/AttrParsedAttrImpl.inc',                     'Basic/Attr.td',                              '--gen-clang-attr-parsed-attr-impl -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'Sema/AttrParsedAttrKinds.inc',                    'Basic/Attr.td',                              '--gen-clang-attr-parsed-attr-kinds -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'Sema/AttrParsedAttrList.inc',                     'Basic/Attr.td',                              '--gen-clang-attr-parsed-attr-list -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'Sema/AttrSpellingListIndex.inc',                  'Basic/Attr.td',                              '--gen-clang-attr-spelling-index -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'Sema/AttrTemplateInstantiate.inc',                'Basic/Attr.td',                              '--gen-clang-attr-template-instantiate -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'Serialization/AttrPCHRead.inc',                   'Basic/Attr.td',                              '--gen-clang-attr-pch-read -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'Serialization/AttrPCHWrite.inc',                  'Basic/Attr.td',                              '--gen-clang-attr-pch-write -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'StaticAnalyzer/Checkers/Checkers.inc',            'StaticAnalyzer/Checkers/Checkers.td',        '--gen-clang-sa-checkers -I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/StaticAnalyzer/Checkers")}"' ),
		clang_tablegen( 'Tooling/Syntax/NodeClasses.inc',                  'Tooling/Syntax/Nodes.td',                    '--gen-clang-syntax-node-classes -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'Tooling/Syntax/Nodes.inc',                        'Tooling/Syntax/Nodes.td',                    '--gen-clang-syntax-node-list -I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),

		clang_diag_gen( 'AST' ),
		clang_diag_gen( 'Analysis' ),
		clang_diag_gen( 'Comment' ),
		clang_diag_gen( 'Common' ),
		clang_diag_gen( 'CrossTU' ),
		clang_diag_gen( 'Driver' ),
		clang_diag_gen( 'Frontend' ),
		clang_diag_gen( 'Lex' ),
		clang_diag_gen( 'Parse' ),
		clang_diag_gen( 'Refactoring' ),
		clang_diag_gen( 'Sema' ),
		clang_diag_gen( 'Serialization' ),
	}
	files {
		'third_party/llvm-project/clang/lib/AST/**.cpp',
		'third_party/llvm-project/clang/lib/Analysis/*.cpp', -- No plugins
		'third_party/llvm-project/clang/lib/Basic/**.cpp',
		'third_party/llvm-project/clang/lib/Driver/**.cpp',
		'third_party/llvm-project/clang/lib/Edit/**.cpp',
		'third_party/llvm-project/clang/lib/Format/**.cpp',
		'third_party/llvm-project/clang/lib/Frontend/**.cpp',
		'third_party/llvm-project/clang/lib/Index/**.cpp',
		'third_party/llvm-project/clang/lib/Lex/**.cpp',
		'third_party/llvm-project/clang/lib/Parse/**.cpp',
		'third_party/llvm-project/clang/lib/Rewrite/**.cpp',
		'third_party/llvm-project/clang/lib/Sema/**.cpp',
		'third_party/llvm-project/clang/lib/Serialization/**.cpp',
		'third_party/llvm-project/clang/lib/Tooling/**.cpp',
		'third_party/llvm-project/clang/tools/libclang/**.cpp',
	}

	filter 'toolset:msc'
		buildoptions {
			'/bigobj',
		}
		disablewarnings {
			'4244',
			'4267',
			'4996',
		}
end
