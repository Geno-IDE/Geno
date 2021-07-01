
function tablegen( output, option, input, extraflags )
	return string.format( 'echo TableGen on '..output..' && "%%{path.join(wks.location,"bin",cfg.platform,cfg.buildcfg,"llvm-tablegen")}" "%%{path.join(wks.location,"third_party/llvm-project/llvm/include/llvm/%s")}" -%s -o "%%{path.join(wks.location,"third_party/llvm-project/llvm/include/llvm/%s")}" %s', input, option, output, extraflags or '' )
end

function clang_tablegen( output, option, input, extraflags )
	return string.format( 'echo Clang TableGen on '..output..' && "%%{path.join(wks.location,"bin",cfg.platform,cfg.buildcfg,"llvm-clang-tablegen")}" "%%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/%s")}" -%s -o "%%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/%s")}" %s', input, option, output, extraflags or '' )
end

function clang_diag_gen( component )
	return 'echo Clang TableGen on Basic/Diagnostic'..component..'Kinds.inc && "%{path.join(wks.location,"bin",cfg.platform,cfg.buildcfg,"llvm-clang-tablegen")}" "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic/Diagnostic.td")}" --gen-clang-diags-defs --clang-component='..component..' -o "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic/Diagnostic'..component..'Kinds.inc")}" -I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"'
end

return function()
	dependson {
		'llvm-tablegen',
	}
	sysincludedirs {
		'third_party/.config',
		'third_party/llvm-project/llvm/include',
		'third_party/llvm-project/clang/include',
	}
	files {
		'third_party/llvm-project/clang/tools/libclang/**.cpp',
	}
	prebuildcommands {
		clang_tablegen( 'AST/Attrs.inc',                               '-gen-clang-attr-classes',                            'Basic/Attr.td',                              '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/AttrImpl.inc',                            '-gen-clang-attr-impl',                               'Basic/Attr.td',                              '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/AttrTextNodeDump.inc',                    '-gen-clang-attr-text-node-dump',                     'Basic/Attr.td',                              '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/AttrNodeTraverse.inc',                    '-gen-clang-attr-node-traverse',                      'Basic/Attr.td',                              '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/AttrVisitor.inc',                         '-gen-clang-attr-ast-visitor',                        'Basic/Attr.td',                              '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/StmtNodes.inc',                           '-gen-clang-stmt-nodes',                              'Basic/StmtNodes.td',                         '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/DeclNodes.inc',                           '-gen-clang-decl-nodes',                              'Basic/DeclNodes.td',                         '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/TypeNodes.inc',                           '-gen-clang-type-nodes',                              'Basic/TypeNodes.td',                         '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/AbstractBasicReader.inc',                 '-gen-clang-basic-reader',                            'AST/PropertiesBase.td',                      '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/AbstractBasicWriter.inc',                 '-gen-clang-basic-writer',                            'AST/PropertiesBase.td',                      '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/AbstractTypeReader.inc',                  '-gen-clang-type-reader',                             'AST/TypeProperties.td',                      '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/AbstractTypeWriter.inc',                  '-gen-clang-type-writer',                             'AST/TypeProperties.td',                      '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/CommentNodes.inc',                        '-gen-clang-comment-nodes',                           'Basic/CommentNodes.td',                      '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/CommentHTMLTags.inc',                     '-gen-clang-comment-html-tags',                       'AST/CommentHTMLTags.td',                     '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/CommentHTMLTagsProperties.inc',           '-gen-clang-comment-html-tags-properties',            'AST/CommentHTMLTags.td',                     '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/CommentHTMLNamedCharacterReferences.inc', '-gen-clang-comment-html-named-character-references', 'AST/CommentHTMLNamedCharacterReferences.td', '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/CommentCommandInfo.inc',                  '-gen-clang-comment-command-info',                    'AST/CommentCommands.td',                     '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/CommentCommandList.inc',                  '-gen-clang-comment-command-list',                    'AST/CommentCommands.td',                     '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'AST/StmtDataCollectors.inc',                  '-gen-clang-data-collectors',                         'AST/StmtDataCollectors.td',                  '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),

		tablegen( 'Frontend/OpenMP/OMP.h.inc', '-gen-directive-decl', 'Frontend/OpenMP/OMP.td', '-I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),
		tablegen( 'Frontend/OpenMP/OMP.inc',   '-gen-directive-gen',  'Frontend/OpenMP/OMP.td', '-I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),

		clang_diag_gen( 'Analysis' ),
		clang_diag_gen( 'AST' ),
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

		clang_tablegen( 'Basic/DiagnosticGroups.inc',         '-gen-clang-diag-groups',                  'Basic/Diagnostic.td', '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"' ),
		clang_tablegen( 'Basic/DiagnosticIndexName.inc',      '-gen-clang-diags-index-name',             'Basic/Diagnostic.td', '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"' ),
		clang_tablegen( 'Basic/AttrList.inc',                 '-gen-clang-attr-list',                    'Basic/Attr.td',       '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'Basic/AttrSubMatchRulesList.inc',    '-gen-clang-attr-subject-match-rule-list', 'Basic/Attr.td',       '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'Basic/AttrHasAttributeImpl.inc',     '-gen-clang-attr-has-attribute-impl',      'Basic/Attr.td',       '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'Basic/arm_neon.inc',                 '-gen-arm-neon-sema',                      'Basic/arm_neon.td',   '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"' ),
		clang_tablegen( 'Basic/arm_fp16.inc',                 '-gen-arm-neon-sema',                      'Basic/arm_fp16.td',   '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"' ),
		clang_tablegen( 'Basic/arm_mve_builtins.inc',         '-gen-arm-mve-builtin-def',                'Basic/arm_mve.td',    '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"' ),
		clang_tablegen( 'Basic/arm_mve_builtin_cg.inc',       '-gen-arm-mve-builtin-codegen',            'Basic/arm_mve.td',    '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"' ),
		clang_tablegen( 'Basic/arm_mve_builtin_sema.inc',     '-gen-arm-mve-builtin-sema',               'Basic/arm_mve.td',    '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"' ),
		clang_tablegen( 'Basic/arm_mve_builtin_aliases.inc',  '-gen-arm-mve-builtin-aliases',            'Basic/arm_mve.td',    '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"' ),
		clang_tablegen( 'Basic/arm_sve_builtins.inc',         '-gen-arm-sve-builtins',                   'Basic/arm_sve.td',    '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"' ),
		clang_tablegen( 'Basic/arm_sve_builtin_cg.inc',       '-gen-arm-sve-builtin-codegen',            'Basic/arm_sve.td',    '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"' ),
		clang_tablegen( 'Basic/arm_sve_typeflags.inc',        '-gen-arm-sve-typeflags',                  'Basic/arm_sve.td',    '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"' ),
		clang_tablegen( 'Basic/arm_sve_sema_rangechecks.inc', '-gen-arm-sve-sema-rangechecks',           'Basic/arm_sve.td',    '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"' ),
		clang_tablegen( 'Basic/arm_cde_builtins.inc',         '-gen-arm-cde-builtin-def',                'Basic/arm_cde.td',    '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"' ),
		clang_tablegen( 'Basic/arm_cde_builtin_cg.inc',       '-gen-arm-cde-builtin-codegen',            'Basic/arm_cde.td',    '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"' ),
		clang_tablegen( 'Basic/arm_cde_builtin_sema.inc',     '-gen-arm-cde-builtin-sema',               'Basic/arm_cde.td',    '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"' ),
		clang_tablegen( 'Basic/arm_cde_builtin_aliases.inc',  '-gen-arm-cde-builtin-aliases',            'Basic/arm_cde.td',    '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/Basic")}"' ),

		tablegen( '../../../clang/include/clang/Driver/Options.inc', '-gen-opt-parser-defs', '../../../clang/include/clang/Driver/Options.td', '-I "%{path.join(wks.location,"third_party/llvm-project/llvm/include")}"' ),

		clang_tablegen( 'Sema/AttrTemplateInstantiate.inc', '-gen-clang-attr-template-instantiate', 'Basic/Attr.td', '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'Sema/AttrParsedAttrList.inc',      '-gen-clang-attr-parsed-attr-list',     'Basic/Attr.td', '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'Sema/AttrParsedAttrKinds.inc',     '-gen-clang-attr-parsed-attr-kinds',    'Basic/Attr.td', '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'Sema/AttrSpellingListIndex.inc',   '-gen-clang-attr-spelling-index',       'Basic/Attr.td', '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),
		clang_tablegen( 'Sema/AttrParsedAttrImpl.inc',      '-gen-clang-attr-parsed-attr-impl',     'Basic/Attr.td', '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include")}"' ),

		clang_tablegen( 'StaticAnalyzer/Checkers/Checkers.inc', '-gen-clang-sa-checkers', 'StaticAnalyzer/Checkers/Checkers.td', '-I "%{path.join(wks.location,"third_party/llvm-project/clang/include/clang/StaticAnalyzer/Checkers")}"' ),
	}

	filter 'toolset:msc'
		disablewarnings {
			'4244',
			'4996',
		}
end
