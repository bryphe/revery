/**
 * write
 *
 * Takes a [path] and writes [data] to that file.
 *
 * Examples:
 * let result: Lwt.t(result(unit, string)) = File.write(~path="example.txt", "Hello World!");
 */
let write: (~path: string, string) => Lwt.t(result(unit, string));

/**
 * delete
 *
 * Takes a [path] and tries to delete that file.
 *
 * Examples:
 * let result: Lwt.t(result(unit, string)) = File.delete("example.txt");
 */
let delete: string => Lwt.t(result(unit, string));
