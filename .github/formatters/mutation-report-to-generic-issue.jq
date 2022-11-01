# This jq filter transforms a mutation testing elements report to the SonarQube generic issue import format.

.framework.name as $frameworkName
| .projectRoot as $projectRoot
| .files
| to_entries
| {
    issues: map(
        .value.mutants[] as $mutants
        | del(.value) as $file
        | $mutants
        | select(.status == ("Survived", "NoCoverage"))
        | (
            if .replacement then
                "The mutation operator '" + .mutatorName + "' has mutated the input to " + .replacement + " without any tests failing."
            else
                "The mutation operator '" + .mutatorName + "' has mutated the input without any tests failing."
            end
        ) as $mutation
        | {
            engineId: ($frameworkName // "Mutation Testing"),
            ruleId: ("Mutant" + .status),
            primaryLocation: {
                message: (
                    if .status == "NoCoverage" then
                        "A mutant was not covered by any of the tests. " + $mutation
                    else
                        "A mutant survived after running the tests. " + $mutation
                    end
                ),
                filePath: (
                    if $ARGS.named["workspace"] != null then
                        $file.key | sub("^" + $ARGS.named["workspace"] + "/"; "")
                    elif $projectRoot then
                        $file.key | sub("^" + $projectRoot + "/"; "")
                    else
                        $file.key
                    end
                ),
                textRange: {
                    startLine: .location.start.line,
                    endLine: .location.end.line,
                    startColumn: (.location.start.column - 1),
                    endColumn: (.location.end.column - 1)
                }
            },
            type: "CODE_SMELL",
            severity: "MAJOR",
            effortMinutes: 10
        }
    )
}
