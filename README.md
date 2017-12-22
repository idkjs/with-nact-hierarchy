# REASON-NACT Tutorial

Hello! This project allows you to quickly get started with Reason and BuckleScript. If you wanted a more sophisticated version, try the `react` template (`bsb -theme react -init .`).

# Build
```
npm run build
```

# Build + Watch

```
npm run watch
```

# Editor
If you use `vscode`, Press `Windows + Shift + B` it will build automatically

## NACT Hierarchy Overview

- Actos are arranged hierarchically
- They can create child actors of their own
- Every actor has a parent
- The lifecycle of an actor is tied to its parent therefore if a parent actor stops, all its children actors stop.
- By exploiting the actor hierarchy, you can enforce seperation of concerns and encapsulate system functionality which help deal with system failures and system shutdown.


## Refactoring Contacts Module to use Hierarchy

- Goal is to support multiple users of the contacts module. We need to do 3 things to achieve this.
- 1. Create a parent to route requests/message to thh correct child see [createContactsService](https://github.com/ncthbrt/reason-nact/blob/a4e316a03910a881c253dadcf8e884d7138157cf/examples/multi_user_contacts.re#L84) code block.
- 2. Modify Contacts.re so we can parameterize its parent and name
- 3. Add a user id to the path of each API endpoint and add a userId into each message.


## creating actors generally

- steps open Nact; 
- define type that actor will expect if not defined elsewhere
- call system to start it actor `let system = start()`
- call spawn type of function, spawnStateless for stateless actors or just spawn for spawnStateful.
- 1st arg is optional name of actor as string, `~name="someActor"`
- 2nd arg is unnamed reference to 'system'
- 3rd arg is unnamed arg that takes a ref to a defined type that is expected, the "_" to call the actor function with follows after fat arrow. oCaml will infer the type if it can.
- call the actor with dispatch(actor, input) or using Nact.Operator syntax `actor <-< { sometype: value }` 
- in stateful components, the first arg to the execution function is state, 2nd is type ref, and the third is the context/ctx.

## greeter.re

- to create an actor you have to spawn it, this example has no stateso we are using spawnStateless(). First unnamed arg to spawnStateless is the parent, here the actor system or "system", unnamed b/c no ~. 2nd '_'/() arg is invoked when a message   is recieved. Then named arg, ~name is optional, the system will assign a name to the arg if it is omitted, dispatch is used to communicate with greeter.

## statefulGreeter.re

- statefulGreeter example demonstrate how its easy to create safe stateful services
- we create a stateful service by calling a spawn function. first initiate with an empty state object. Each time a msg is recieved, the current state is passed as the first arg to the actor. if new name is added, the actor will copy the current state then add the new name to it. The return value is used as the next state.

## Js.Promise.resolve
- see: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Promise/resolve
- The Promise.resolve(value) method returns a Promise object that is resolved with the given value.

## Querying with NACT
- see: https://github.com/ncthbrt/reason-nact#querying

- NACT is a non-blocking system while REST and RPC systems on the frontend are blocking, so NACT provides a Query method to bridge the gap.

- Similar to `dispatch`, `query` pushes and msg to an actors mailbox but differs by also creating a temporary actor which is passed into a function which the message to return to the target actor. When the temp actor recieves a message, the promised recieved by the query resolves.

## Query Timeouts

- In addition to the message, query also takes in a timeout value measured in milliseconds. If a query takes longer than this time to resolve, it times out and the promise is rejected. A time bounded query is very important in a production system; it ensures that a failing subsystem does not cause cascading faults as queries queue up and stress available system resources.

## Address Book with NACT Queries

- AddressBook API requirements: 
- Be able to create a new contact
- Be able to fetch all contacts
- Be able to fetch a specifiec contact
- Be able to update an existing contact
- Be able to delete a contact

## Address Book Steps

- actors are message driven, define message types we will need to accomplish the API requirements. We will need a contactId, contact, contactResponseMsg, and a contactMsg.

- message shape: we need to know the shape of the contact actor state.

- we need functions to handle each message type.

- after we do all of the above steps, we will have the infrastructure to create an actor.