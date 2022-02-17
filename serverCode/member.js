const mongoose = require('mongoose');

const MemberSchema = mongoose.Schema({
    idtbl_member: {
        type: Number
    },
    student_id: {
        type: String,
        required: true
    },
    role: {
        type: String
    },
    ssh_profile: {
        type: String
    },
    chinese_name: {
        type: String,
        required: true
    },
    created_timestamp: {
        type: String,
        required: true
    },
    display_name: {
        type: String
    },
    fingerprint_id: {
        type: Number
    },
    birthday: {
        type: String
    },
    face_id: {
        type: Number
    },
    slack_userID:{
        type: String
    },
    slack_email:{
        type: String
    },
    slack_image:{
        type: String
    },
    doorbot_frontend_psw:{
        type: String,
    }
});

module.exports = mongoose.model('member',MemberSchema,'members')