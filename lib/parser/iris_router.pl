%% iris_router.pl - Logical command router for Iris CLI
%% Supports: multiline/bracket args [git push], && / || / not, multiparameter and prefix commands.
%% Loaded from C via consult/1; predicates are in user module.
%%
%% iris_goals(ArgvList, GoalList)
%%   ArgvList: list of atoms from C argv (e.g. [iris, alias, add, [, git, push, ], &&, iris, copush])
%%   GoalList: list of goal(Op, Command)
%%     Op = and | or  (connective to previous goal)
%%     Command = help | init | commit | rebuild | not(Inner) | alias_add(ArgList)
%%   C executes the chain: and = run and continue only on success; or = run and stop on success.

:- multifile command_alias/2.

%% --- Main entry: parse argv into list of goals ---
iris_goals(Argv, GoalList) :-
    split_by_connectives(Argv, Segs),
    maplist(seg_to_goal, Segs, GoalList).

%% Split at '&&' and '||'. Each segment is (Op, Tokens); Op is the connective
%% that precedes this segment (and/or). First segment gets 'and'.
split_by_connectives(Argv, Segs) :-
    split_at_connectives(Argv, Segs).

split_at_connectives([], []).
split_at_connectives(Argv, [(Op, Seg)|Rest]) :-
    take_until_connective(Argv, Seg, Conn, Tail),
    Seg \= [],
    ( Conn = end -> Rest = [], Op = and
    ; Conn = and -> Op = and, split_at_connectives(Tail, Rest)
    ; Conn = or  -> Op = or,  split_at_connectives(Tail, Rest)
    ),
    !.
split_at_connectives(Argv, [(and, Argv)]) :- Argv \= [].

take_until_connective([], [], end, []).
take_until_connective(['&&'|T], [], and, T).
take_until_connective(['||'|T], [], or, T).
take_until_connective([H|T], [H|Seg], Conn, Tail) :-
    H \= '&&', H \= '||',
    take_until_connective(T, Seg, Conn, Tail).

seg_to_goal((Op, Toks), goal(Op, Cmd)) :-
    segment_to_command(Toks, Cmd).

segment_to_command(Toks, Cmd) :-
    drop_program_name(Toks, Rest),
    group_brackets(Rest, Grouped),
    parse_command(Grouped, Cmd).

drop_program_name([], []).
drop_program_name([_|T], T).  % drop first token (program name, e.g. iris)

%% Group tokens between [ and ] into a single list element.
group_brackets([], []).
group_brackets(['['|T], [List|Rest]) :-
    take_until_closing(T, List, Remain),
    group_brackets(Remain, Rest).
group_brackets([H|T], [H|Rest]) :-
    H \= '[',
    group_brackets(T, Rest).

take_until_closing([], [], []).
take_until_closing([']'|T], [], T).
take_until_closing([H|T], [H|L], R) :-
    H \= ']',
    take_until_closing(T, L, R).

%% Parse one segment: optional leading 'not', then command (simple or prefix+params).
parse_command([], help).
parse_command([_], help).
parse_command([not|Rest], not(Inner)) :-
    parse_command(Rest, Inner),
    !.
parse_command([alias, add|Args], alias_add(Args)) :- !.
parse_command([alias|_], help) :- !.
parse_command([Cmd|_], Command) :-
    command_alias(Cmd, Command),
    !.
parse_command([_|_], help).

%% Backward compatibility: single command (no chain); unwrap not(.) for legacy callers
iris_command(Argv, Command) :-
    iris_goals(Argv, [goal(_, Cmd)|_]),
    ( Cmd = not(_) -> Command = help  % legacy: not as unknown
    ; Cmd = alias_add(_) -> Command = help
    ; Command = Cmd
    ),
    !.
iris_command(Argv, help) :-
    iris_goals(Argv, []),
    !.
iris_command(_, help).

%% Aliases: map CLI tokens to canonical command atoms
command_alias(init,   init).
command_alias(commit, commit).
command_alias(copush, commit).
command_alias(rebuild, rebuild).
