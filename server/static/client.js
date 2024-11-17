const remove_link = document.getElementById("remove")

remove_link.addEventListener("click", (e)=>{
  let res = confirm(
    "when you remove a client, it will self-destruct itself after the next connection, "+
    "do you want to continue?"
  )

  if(!res)
    e.preventDefault()
})
