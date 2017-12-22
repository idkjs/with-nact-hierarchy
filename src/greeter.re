/* import start function which start then returns the actor system,
   provides reference to nact system */
open Nact;

let system = start();

type greetingMsg = {name: string};

let greeter =
  spawnStateless(
    ~name="greeter",
    system,
    ({name}, _) => print_endline("Hello " ++ name) |> Js.Promise.resolve
  );

/* dispatch is used to communicate with greeter. */
/* dispatch(greeter, {name: "Erlich Bachman"}); */
/* dispatching with Nact.Operators's */
open Nact.Operators;

/* these are the same. they do the same thing */
/* ex 1 */
greeter <-< {name: "Erlich Bachman"};
/* ex 2 */
/* {name: "Erlich Bachman"} >-> greeter; */