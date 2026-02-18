%% iris_router.pl - Logical command router for Iris CLI
%% Resolves argv (list of atoms) to a single command atom: help, init, commit, rebuild.
%% Loaded from C via consult/1; predicates are in user module.

%% iris_command(ArgvList, Command)
%%   ArgvList: list of atoms as from C argv (e.g. [iris, init] or [iris, commit])
%%   Command:  one of help, init, commit, rebuild
iris_command(Argv, help) :-
    ( Argv = []
    ; Argv = [_]
    ),
    !.
iris_command([_|Rest], Command) :-
    Rest = [CmdAtom|_],
    command_alias(CmdAtom, Command),
    !.
iris_command([_|_], help) :- !.  % unknown command -> help

%% Aliases: map CLI tokens to canonical command atoms
command_alias(init,   init).
command_alias(commit, commit).
command_alias(copush, commit).
command_alias(rebuild, rebuild).
