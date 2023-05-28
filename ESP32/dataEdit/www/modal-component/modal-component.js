const modalTemplate = document.createElement("template");
modalTemplate.innerHTML = `
<style>
    .modal-overlay {
        position: fixed;
        z-index: 50;
        width: 100vw;
        height: 100vh;
        top: 0;
        background: rgb(0.5,0.5,0.5,0.5);
    }
    .modal-container {
        display: flex;
        flex-flow: column !important;
        flex-wrap: nowrap;
        overflow-y: hidden;
        max-height: 100vh;
        max-width: 85vw;
        margin: auto;
        background-color: black;
        border: 1px solid white;
        border-radius: 25px;
    }
    .modal-header {
        display: flex;
        justify-content: space-between;
    }
    .modal-close-button {
        margin: 16px;
        border-radius: 25px;
        width: 30px;
        height: 30px;
        text-align: center;
    }
    .modal-body {
        display: flex;
        justify-content: center;
        overflow-y: auto;
    }
    .modal-container {
        display: flex;
        flex-flow: row;
        flex-wrap: nowrap;
        justify-content: space-between;
    }

</style>
<div class="modal-overlay">
    <div class="modal-container">
        <div class="modal-header">
            <h2>
                <slot name="title"></slot>
            </h2>
            <div><button class="modal-close-button" onclick="this.getRootNode().host.hide()">X</button></div>
        </div>
        <div class="modal-body">
            <slot></slot>
        </div>
        <div class="modal-footer">
            <div>
                <slot name="footerStart"></slot>
            </div>
            <div>
                <slot name="footerEnd"></slot>
            </div>
        </div>
    </div>
</div>
`

class ModalComponent extends HTMLElement {
    constructor() {
        super();
        const shadowRoot = this.attachShadow({mode: 'open'});
        shadowRoot.append(modalTemplate.content.cloneNode(true));
        
        //this.classList.add('hidden');
    }
    
    static get observedAttributes () {
        return ["visible"];
    }

    attributeChangedCallback(name, oldeValue, newValue) {
        if(name == "visible") {
            if(newValue) {
                this.show();
            } else {
                this.hide();
            }
        }
    } 
    connectedCallback() {
        // browser calls this method when the element is added to the document
        // (can be called many times if an element is repeatedly added/removed)
        console.log("connected");
    }

    disconnectedCallback() {
        // browser calls this method when the element is removed from the document
        // (can be called many times if an element is repeatedly added/removed)
        console.log("disconnected");
    }

    adoptedCallback() {
        // called when the element is moved to a new document
        // (happens in document.adoptNode, very rarely used)
        console.log("adopted");
    }

    show() {
        this.classList.remove('hidden');
    }
    hide() {
        this.classList.add('hidden');
        this.removeAttribute("visible");
    }
};

customElements.define('modal-component', ModalComponent);