% (included file)

:- doc(section, "Ant Options").

% TODO: should it be configurable?
:- bundle_flag(with_ant, [
    comment("Enable Ant support"),
    details(
      % .....................................................................
      "Set to \"yes\" if you wish to enable Ant support.\n"),
    valid_values(['yes', 'no']),
    %
    default_comment("Ant detected"),
    default_value_comment(no,
        "Ant has not been detected.\n"||
        "'ciaopp/ilciao' needs 'ant' as a build dependency."),
    rule_default(VerifyAnt, (
      flag(with_java_interface(WithJavaInterface)),
      verify_ant(WithJavaInterface, VerifyAnt))),
    %
    interactive([advanced])
]).

verify_ant(no,  no).
verify_ant(yes, VerifyAnt) :-
	verify_ant_yes(VerifyAnt).

verify_ant_yes(Value) :-
	( ant_installed -> Value = yes ; Value = no ).

ant_installed :- find_executable('ant', _).

