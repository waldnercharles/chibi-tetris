def revision: type == "string" and test("^[0-9a-f]{40}$");

(if has("compatible_engines") then .compatible_engines else [] end) as $compatible |
.format == 1 and
(.engine | revision) and
($compatible | type == "array" and all(.[]; revision)) and
(.engine == $engine or ($compatible | index($engine)) != null) and
(.studio | revision) and
(.archive | type == "string" and test("^[A-Za-z0-9_.-]+\\.tar\\.gz$")) and
(.sha256 | type == "string" and test("^[0-9a-f]{64}$"))
