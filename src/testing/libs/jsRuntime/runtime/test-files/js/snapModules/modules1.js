import {module2Int} from './module2.js';

export const module1 = {
    function1: ()=> {
        return 42;
    },

    function2: ()=>{
        return module2Int;
    }
}