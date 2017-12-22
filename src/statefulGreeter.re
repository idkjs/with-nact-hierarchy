/* this example wont do anything because we dont call dispatch, just stateful example */
/* - steps open Nact;
   - define type that actor will expect if not defined elsewhere
   - call system to start it actor `let system = start()`
   - call spawn type of function, spawnStateless for stateless actors
   or just spawn for spawnStateful.
   - 1st arg is optional name of actor as string, `~name="someActor"`
   - 2nd arg is unnamed reference to 'system'
   - 3rd arg:stateful, first arg to this function is state in stateful actors, second takes ref to a defined type that is expected, the "_" to
   call the actor function with follows after fat arrow. oCaml will infer the type if it can.
   - call the actor with dispatch(actor, input) or
   using Nact.Operator syntax `actor <-< { sometype: value }`
   - in stateful components, the first arg to the execution function is state, 2nd is type ref,
   and the third is the context/ctx.
   */
open Nact;

type greetingMsg = {name: string};

let system = start();

let statefulGreeter =
  spawn(
    ~name="stateful-greeter",
    system,
    (state, {name}, ctx) => {
      /* define previous state */
      let hasPreviouslyGreetedMe = List.exists((v) => v === name, state);
      if (hasPreviouslyGreetedMe) {
        print_endline("Hello Again " ++ name);
        state |> Js.Promise.resolve
      } else {
        print_endline("Good to meet you, " ++ name ++ ". I am the " ++ ctx.name ++ " service!");
        /* return the new name and current state, resolve */
        [name, ...state] |> Js.Promise.resolve
      }
    },
    []
  );