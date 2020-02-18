function updateObjData() {
    obj_data.data.name = data_name.text
    _updateDate()
}

function _updateDate() {
    // Parse & simple check date
    if( wdate.checkFormat(data_birth_date.text, "dd/MM/yyyy") )
        obj_data.data.birth_date = wdate.unixtimeFromString(data_birth_date.text, "dd/MM/yyyy", true)
    else {
        obj_data.data.birth_date = null
        data_birth_date.text = ""
    }

    if( wdate.checkFormat(data_death_date.text, "dd/MM/yyyy") ) {
        obj_data.data.death_date = wdate.unixtimeFromString(data_death_date.text, "dd/MM/yyyy", true)
    } else {
        obj_data.data.death_date = null
        data_death_date.text = ""
    }

    if( obj_data.data.birth_date !== null && obj_data.data.death_date !== null && obj_data.data.death_date < obj_data.data.birth_date ) {
        console.log("death < birth")
        obj_data.data.death_date = null
        data_death_date.text = ""
    }
}
