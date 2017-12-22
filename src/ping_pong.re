/* -  open Nact; and/or Nact.Operators;
   - call system to start it actor `let system = start()`
   - define type that actor will expect if not defined elsewhere

   - define actor function and call spawn type of function, spawnStateless for stateless actors
   or just spawn for spawnStateful.
   - 1st arg is optional name of actor as string, `~name="someActor"`
   - 2nd arg is unnamed reference to 'system'
   - 3rd arg is unnamed arg that takes a ref to a defined type that is expected, the "_" to
   call the actor function with follows after fat arrow. oCaml will infer the type if it can.
   - call the actor with dispatch(actor, input) or
   using Nact.Operator syntax `actor <-< { sometype: value }`
   - in stateful components, the first arg to the execution function is state, 2nd is type ref,
   and the third is the context/ctx.
   */
/* An actor alone is a somewhat useless construct; actors need to work together.
   Actors can send messages to one another by using the dispatch method.
   In this example, the actors Ping and Pong are playing a perfect ping-pong match.
   To start the match, we dispatch a message to ping and specify that the sender
   in msgType is pong. */
open Nact;

open Nact.Operators;

let system = start();

/* define type actor will take, here type is msgType, constructor is Msg and it expects
   an actorRef/action(?) and a string as args*/
type msgType =
  | Msg(actorRef(msgType), string);

/* define the actor and type it as actorRef(msgType),  this defines what happens
   when we use the ping actor. We want to call message, use ping as the sender and provide
   a message to send from this ping context. So the return object on the () will print
   whatever message we pass it when we call the defined function.  */
let ping: actorRef(msgType) =
  spawnStateless
    /* 1. optional name of actor */
    (
      ~name="ping",
      system,
      /* 3rd arg is () that does what we want to do with this actor. It takes a ref to a defined type, here its MsgType and we will
         pass in two args, the actorRef val will be sender and the string val will be the
         message */
      (Msg(sender, msg), ctx) => {
        /* what do we want it to do */
        print_endline(msg);
        /* tell it to do it using Nact.Operators syntax. Here, we are saying
           sender is the actorReference, person who is sending the msg, the calling Msg,
           saying ctx.self is the sender, its name is ping */
        sender <-< Msg(ctx.self, ctx.name) |> Js.Promise.resolve
      }
    );

/* define a second actor who will communicate with ping */
let pong: actorRef(msgType) =
  spawnStateless
    /* 1. optional name of actor */
    (
      ~name="pong",
      system,
      /* 3rd arg is () that does what we want to do with this actor. It takes a ref to a defined type, here its MsgType and we will
         pass in two args, the actorRef val will be sender and the string val will be the
         message */
      (Msg(sender, msg), ctx) => {
        /* what do we want it to do */
        print_endline(msg);
        /* tell it to do it using Nact.Operators syntax. Here, we are saying
           get the actorRef, person who is sending the msg, */
        sender <-< Msg(ctx.self, ctx.name) |> Js.Promise.resolve
      }
    );

/* ping is sending a message to pong that says hello. pong is a function, so when pong
   gets a message, he responds to whoever sent the message, here, ping, with his name, pong.
   Keeps going in a loop
   */
ping <-< Msg(pong, "hello");