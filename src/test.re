/* open Nact;

   open Contacts;

   let createDinesh =
     query(
       ~timeout=100 * milliseconds,
       contactsService,
       (tempReference) => (
         tempReference,
         CreateContact({name: "Dinesh Chugtai", email: "dinesh@piedpiper.com"})
       )
     );

   createDinesh |> Js.log; */