var jquery = require("jquery");
window.$ = window.jQuery = jquery;

function switchContainer(container_name) {
    var active_container = $(`#${container_name}_container`);
    if (active_container.data('active') == 'true') { return; }
    $(".outer-container").animate({ opacity: 0 }, 250).css('display', 'none').removeData("active");
    active_container.css('display', 'flex').data("active", "true").animate({ opacity: 1 }, 250);

    var activation_function = active_container.data("activate");
    if (activation_function) {
        eval(activation_function);
    }
}

function formatHz(value) {
    if (value >= 1000000) {
        return `${value / 1000000.0}MHz`;
    } else if (value >= 1000) {
        return `${value / 1000.0}kHz`;
    } else {
        return `${value}Hz`;
    }
}

function loadConfig() {
    $.get("/config").done(config_raw => {
        var config = JSON.parse(config_raw);
        $("*[name='ssid']").val(config.ssid);
        $("*[name='local_ip']").val(config.local_ip);
        $("*[name='gateway']").val(config.gateway);
        $("*[name='subnet']").val(config.subnet);
    });
}

$(document).ready(function(){
    M.AutoInit();
    $("*[name='resolution']").trigger("change");
    $("*[name='clk_div']").trigger("change");

    var active_container = "index";
    switchContainer(active_container);

    $(".navigator").on('click', function(event){
        var active_container = $(this).attr("data-container");
        switchContainer(active_container);
    });

    $("button[type='reset']").on('click', function(event){
        event.preventDefault();
        $(this).parents("form").trigger("reset"); // forcing reset event
        M.updateTextFields(); //updating labels from inputs
    });

    $("button[id='stopSquare']").on('click', function(event){
        var params = {
            type: "square",
            pwm_channel: $("*[name='pwm_channel']").val(),
            highspeed: $("*[name='highspeed']").is(":checked")
        };
        $.post("/stop", $.param(params)).done((res) => console.log(res));
    });
    
    $("button[id='stopSine']").on('click', function(){
        var params = {
            type: "sine",
            dac_channel: $("*[name='dac_channel']").val()
        };
        $.post("/stop", $.param(params)).done((res) => console.log(res));
    });

    $("*[name='resolution']").change(function(event) {
        var resolution = parseInt($(this).val());
        var max_freq = Math.round(80000000.0 / Math.pow(2, resolution));
        $("*[name='pwm_frequency']").attr("max", max_freq).siblings(".helper-text").html(`Min: 1Hz / Max: ${formatHz(max_freq)}`);
    });

    $("*[name='clk_div']").change(function(event) {
        var clk_div = parseInt($(this).val());
        var min_freq = Math.round(135.0 / (clk_div + 1.0));
        var max_freq = Math.round(135000.0 / (clk_div + 1.0));
        $("*[name='dac_frequency']").attr("min", min_freq).attr("max", max_freq * 10).siblings(".helper-text").html(`Min: ${formatHz(min_freq)} / Max: ${formatHz(max_freq)} (without distortion)`);
    });

    $("#square-setup-form").submit(function(event) {
        // Stop form from submitting normally
        event.preventDefault();
        
        // Get some values from elements on the page:
        var $form = $(this);
        
        console.debug($form.serialize());
        
        var resolution = parseInt($("*[name='resolution']").val());
        var max_value = Math.pow(2, resolution) - 1;
        
        var config = {
            type: "square",
            timer_num: $("*[name='timer_num']").val(),
            pwm_channel: $("*[name='pwm_channel']").val(),
            highspeed: $("*[name='highspeed']").is(":checked"),
            resolution: resolution,
            frequency: $("*[name='pwm_frequency']").val(),
            duty: Math.round(max_value / 100.0 * $("*[name='duty']").val()),
            phase: Math.round($("*[name='pwm_phase']").val() / 360.0 * max_value),
            out_pin: $("*[name='out_pin']").val()
        };
        
        console.debug(config);
        
        $.post("/setup", $.param(config)).done((res) => console.log(res));
    });

    $("#sine-setup-form").submit(function(event) {
        // Stop form from submitting normally
        event.preventDefault();
        
        // Get some values from elements on the page:
        var $form = $(this);
        
        console.debug($form.serialize());

        var clk_div = parseInt($("*[name='clk_div']").val());
        var min_freq = 135.0 / (clk_div + 1.0);
        var frequency = Math.round($("*[name='dac_frequency']").val() / min_freq);
        
        var config = {
            type: "sine",
            dac_channel: $("*[name='dac_channel']").val(),
            clk_div,
            invert: $("*[name='invert']").val(),
            frequency,
            phase: Math.round($("*[name='dac_phase']").val() / 360.0 * 255.0),
            scale: $("*[name='scale']").val()
        };
        
        console.debug(config);
        
        $.post("/setup", $.param(config)).done(function(res) {
            console.log(res);
        });
    });

    $("#config-form").submit(function(event) {
        // Stop form from submitting normally
        event.preventDefault();
        
        // Get some values from elements on the page:
        var $form = $(this);
        
        console.debug($form.serialize());
        
        var config = {
            ssid: $("*[name='ssid']").val(),
            password: $("*[name='password']").val(),
        };
    
        var local_ip = $("*[name='local_ip']").val();
        if (local_ip) {
            config.local_ip = local_ip;
        }
        var gateway = $("*[name='gateway']").val();
        if (gateway) {
            config.gateway = gateway;
        }
        var subnet = $("*[name='subnet']").val();
        if (subnet) {
            config.subnet = subnet;
        }
        
        console.debug(config);
        
        $.post("/config", $.param(config)).done((res) => console.log(res));
    });

    $("button[id='reboot']").on('click', function(event){
        $.get("/reboot").done((res) => {
            console.log(res);
            window.location.href = "/";
        });
    });
});