#version 330 core
out     vec4        frag_color;

in      vec3        v_local_pos;

uniform sampler3D   u_volume;
uniform sampler1D   u_transfer_func;
uniform vec3        u_local_cam_pos;
uniform vec3        u_ray_origin;
uniform int         u_max_steps;
// uniform float       u_step_size;

void main(){

    vec3 ray_dir = normalize(v_local_pos - u_ray_origin);
    vec3 current_pos = v_local_pos;
    
    vec4 accum_color = vec4(0.0);
    float step_size = 0.002;

    for(int i = 0; i < u_max_steps; i++) {
        
        if (current_pos.x < 0.0 || current_pos.x > 1.0 ||
                    current_pos.y < 0.0 || current_pos.y > 1.0 ||
                    current_pos.z < 0.0 || current_pos.z > 1.0) {
                    break;
                    }

        float density = texture(u_volume, current_pos).r;
        vec4 sampled_color = texture(u_transfer_func, density);

        if(sampled_color.a > 0.0) {
            float alpha = sampled_color.a;
            accum_color.rgb += (1.0 - accum_color.a) * sampled_color.rgb * alpha;
            accum_color.a   += (1.0 - accum_color.a) * alpha;

            if(accum_color.a >= 0.95) {
                break;
            }
        }

        current_pos += ray_dir * step_size;
    }

    if(accum_color.a < 0.01) {
        discard;
    }
    
    frag_color = accum_color;
}