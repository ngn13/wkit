const source_select = document.getElementById("source") 
const custom_input = document.getElementById("custom")

const script_input = document.getElementById("script")
const copy_button = document.getElementById("copy")

function check_option(){
  if(null == custom_input)
    return

  if(source_select.value == "custom")
    custom_input.style.display = "block"
  else
    custom_input.style.display = "none"
}

if (null != source_select) {
  source_select.addEventListener("change", check_option)
  check_option()
}

if (null != script_input && null != copy_button){
  const script_url = script_input.value
  const copy_text = copy_button.innerText 

  copy_button.addEventListener("click", (e)=>{
    e.preventDefault()

    try {
      navigator.clipboard.writeText(script_url)
      copy_button.innerText = "copied URL to the clipboard"
      setTimeout(()=>{copy_button.innerText = copy_text}, 1500)
    }catch {
      alert(`failed to copy URL to the clipboard, so do it yourself: ${script_url}`)
    }
  })
}
