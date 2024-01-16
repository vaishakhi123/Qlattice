from . import rbc_ukqcd as ru
from . import rbc_ukqcd_params as rup
from .jobs import *

# -----------------------------------------------------------------------------

@q.timer
def run_get_inverter(job_tag, traj, *, inv_type, get_gf, get_gt=None, get_eig=None):
    if None in [ get_gf, ]:
        return
    if get_gt is None:
        get_gt = lambda: None
    if get_eig is None:
        get_eig = lambda: None
    gf = get_gf()
    gt = get_gt()
    eig = get_eig()
    for inv_acc in [ 0, 1, 2, ]:
        ru.get_inv(gf, job_tag, inv_type, inv_acc, gt=gt, eig=eig)

# -----------------------------------------------------------------------------

@q.timer_verbose
def compute_prop_full_1(inv, src, *, tag, sfw):
    sol = inv * src
    q.qnorm_field(sol).save_double(sfw, tag + " ; qnorm_field")
    sol.save_double(sfw, tag)
    sfw.flush()
    return sol

@q.timer
def compute_prop_wsrc_full(gf, gt, tslice, job_tag, inv_type, inv_acc, *,
                           idx, sfw, eig, finished_tags):
    tag = f"tslice={tslice} ; type={inv_type} ; accuracy={inv_acc}"
    if tag in finished_tags:
        return None
    q.check_stop()
    q.check_time_limit()
    q.displayln_info(f"compute_prop_wsrc_full: idx={idx} tslice={tslice}", job_tag, inv_type, inv_acc)
    inv = ru.get_inv(gf, job_tag, inv_type, inv_acc, gt=gt, eig=eig)
    total_site = q.Coordinate(get_param(job_tag, "total_site"))
    geo = q.Geometry(total_site, 1)
    src = q.mk_wall_src(geo, tslice)
    prop = compute_prop_full_1(inv, src, tag=tag, sfw=sfw)

@q.timer_verbose
def compute_prop_wsrc_full_all(job_tag, traj, *,
                               inv_type, gf, gt, wi, eig):
    inv_type_names = [ "light", "strange", ]
    inv_type_name = inv_type_names[inv_type]
    path_s = f"{job_tag}/prop-wsrc-full-{inv_type_name}/traj-{traj}"
    finished_tags = q.properly_truncate_fields(get_save_path(path_s + ".acc"))
    sfw = q.open_fields(get_save_path(path_s + ".acc"), "a", q.Coordinate([ 2, 2, 2, 4, ]))
    for inv_acc in [ 2, 1, ]:
        for p in wi:
            idx, tslice, inv_type_p, inv_acc_p=p
            if inv_type_p == inv_type and inv_acc_p == inv_acc:
                compute_prop_wsrc_full(gf, gt, tslice, job_tag, inv_type, inv_acc,
                                       idx=idx, sfw=sfw, eig=eig,
                                       finished_tags=finished_tags)
    sfw.close()
    q.qrename_info(get_save_path(path_s + ".acc"), get_save_path(path_s))

@q.timer
def run_prop_wsrc_full(job_tag, traj, *, inv_type, get_gf, get_eig, get_gt, get_wi):
    if None in [ get_gf, get_gt, ]:
        return
    if get_eig is None:
        if inv_type == 0:
            return
        get_eig = lambda: None
    inv_type_names = [ "light", "strange", ]
    inv_type_name = inv_type_names[inv_type]
    if get_load_path(f"{job_tag}/prop-wsrc-full-{inv_type_name}/traj-{traj}/geon-info.txt") is not None:
        return
    if q.obtain_lock(f"locks/{job_tag}-{traj}-wsrc-full-{inv_type_name}"):
        gf = get_gf()
        gt = get_gt()
        eig = get_eig()
        wi = get_wi()
        compute_prop_wsrc_full_all(job_tag, traj,
                                   inv_type=inv_type, gf=gf, gt=gt, wi=wi,
                                   eig=eig)
        q.release_lock()

# -----------------------------------------------------------------------------

@q.timer
def avg_weight_from_prop_full(geo, prop_nf_dict, weight_min):
    """
    prop_nf_dict[(inv_type, tslice,)] => prop_nf, prop_nf_glb_sum_tslice
    type(prop_nf) = q.FieldRealD
    type(prop_nf_glb_sum_tslice) = q.SelectedPointsRealD
    """
    fname = q.get_fname()
    inv_type_names = [ "light", "strange", ]
    total_site = geo.total_site()
    n_samples = [ 0, 0, ]
    avg_nf_glb_sum_tslice = [ 0, 0, ]
    for k, v in prop_nf_dict.items():
        inv_type, tslice = k
        prop_nf, prop_nf_glb_sum_tslice = v
        gst = np.roll(prop_nf_glb_sum_tslice[:].ravel(), -tslice)
        n_samples[inv_type] += 1
        avg_nf_glb_sum_tslice[inv_type] += gst
    for inv_type in [ 0, 1, ]:
        inv_type_name = inv_type_names[inv_type]
        assert n_samples[inv_type] == total_site[3]
        avg_nf_glb_sum_tslice[inv_type] = avg_nf_glb_sum_tslice[inv_type] / (geo.total_volume() / total_site[3] * n_samples[inv_type])
        q.displayln_info(-1, fname, "avg_nf_glb_sum_tslice", inv_type_name, avg_nf_glb_sum_tslice[inv_type])
    local_tsize = geo.local_site()[3]
    local_t_start = geo.coor_node()[3] * local_tsize
    local_t_end = local_t_start + local_tsize
    local_tslices = slice(local_t_start, local_t_end, 1)
    local_field_shape = tuple(reversed(geo.local_site().to_list()))
    f_weight_avg = []
    for inv_type in [ 0, 1, ]:
        f = q.FieldRealD(geo, 1)
        q.set_zero(f)
        f_weight_avg.append(f)
    for k, v in prop_nf_dict.items():
        inv_type, tslice = k
        prop_nf, prop_nf_glb_sum_tslice = v
        avg_gst = avg_nf_glb_sum_tslice[inv_type][:].ravel()
        avg_gst_local = np.roll(avg_gst, tslice)[local_tslices]
        weight = prop_nf[:].reshape(local_field_shape) / avg_gst_local[:, None, None, None]
        # q.displayln_info(q.avg_err(weight.ravel()))
        view = f_weight_avg[inv_type][:].reshape(local_field_shape)
        view += weight / n_samples[inv_type]
    f_weight_final = q.FieldRealD(geo, 1)
    q.set_zero(f_weight_final)
    for inv_type in [ 0, 1, ]:
        f_weight_final[:] += f_weight_avg[inv_type][:] / 2
        inv_type_name = inv_type_names[inv_type]
    f_weight_final[:] = np.maximum(weight_min, f_weight_final[:])
    return f_weight_avg, f_weight_final

@q.timer
def make_fsel_from_weight(f_weight, f_rand_01, rate):
    fname = q.get_fname()
    geo = f_weight.geo()
    fsel = q.FieldSelection(geo)
    sel = f_weight[:].ravel() * rate >= f_rand_01[:].ravel()
    val = np.rint(f_weight[:].ravel()[sel] * 10**8).astype(int)
    assert np.all(val >= 0)
    fsel[sel] = val
    fsel.update()
    q.displayln_info(-1, f"{fname} rate = {rate} ; expect_num = {geo.total_volume() * rate} ; actual_num = {q.glb_sum(fsel.n_elems())}")
    return fsel

@q.timer
def make_psel_from_weight(f_weight, f_rand_01, rate):
    fsel = make_fsel_from_weight(f_weight, f_rand_01, rate)
    psel = fsel.to_psel()
    return psel

@q.timer_verbose
def run_fsel_psel_from_wsrc_prop_full(job_tag, traj, *, get_wi):
    """
    return get_fsel, get_psel, get_fsel_prob, get_psel_prob, get_f_rand_01
        fsel = get_fsel()
        psel = get_psel()
        fsel_prob = get_fsel_prob()
        psel_prob = get_psel_prob()
        f_rand_01 = get_f_rand_01()
    Or if wsrc_prop_full is not available
    return None
    """
    fname = q.get_fname()
    total_site = q.Coordinate(get_param(job_tag, "total_site"))
    geo = q.Geometry(total_site, 1)
    fn_fsel_weight = f"{job_tag}/field-selection-weight/traj-{traj}/weight.field"
    fn_fsel_rand = f"{job_tag}/field-selection-weight/traj-{traj}/f-rand-01.field"
    fn_fsel = f"{job_tag}/field-selection/traj-{traj}.field"
    fn_psel = f"{job_tag}/point-selection/traj-{traj}.txt"
    fn_fsel_prob = f"{job_tag}/field-selection-weight/traj-{traj}/fsel-prob.sfield"
    fn_psel_prob = f"{job_tag}/field-selection-weight/traj-{traj}/psel-prob.lat"
    @q.lazy_call
    @q.timer_verbose
    def get_fsel_prob():
        fsel = q.FieldSelection()
        total_size = fsel.load(get_load_path(fn_fsel))
        assert total_size > 0
        fsel_prob = q.SelectedFieldRealD(fsel, 1)
        total_size = fsel_prob.load_double(get_load_path(fn_fsel_prob))
        assert total_size > 0
        return fsel_prob
    @q.lazy_call
    @q.timer_verbose
    def get_psel_prob():
        psel = q.PointsSelection()
        psel.load(get_load_path(fn_psel), geo)
        psel_prob = q.SelectedPointsRealD(psel, 1)
        psel_prob.load(get_load_path(fn_psel_prob))
        return psel_prob
    @q.lazy_call
    @q.timer_verbose
    def get_fsel():
        fsel_prob = get_fsel_prob()
        return fsel_prob.fsel
    @q.lazy_call
    @q.timer_verbose
    def get_psel():
        psel_prob = get_psel_prob()
        return psel_prob.psel
    @q.lazy_call
    @q.timer_verbose
    def get_f_rand_01():
        f_rand_01 = q.FieldRealD(geo, 1)
        f_rand_01.load_double(get_load_path(fn_fsel_rand))
        return f_rand_01
    ret = get_fsel, get_psel, get_fsel_prob, get_psel_prob, get_f_rand_01
    if get_load_path(fn_fsel_prob) is not None and get_load_path(fn_psel_prob) is not None:
        assert get_load_path(fn_fsel) is not None
        assert get_load_path(fn_psel) is not None
        assert get_load_path(fn_fsel_rand) is not None
        assert get_load_path(fn_fsel_weight) is not None
        return ret
    inv_type_names = [ "light", "strange", ]
    path_f_list = [ f"{job_tag}/prop-wsrc-full-{inv_type_name}/traj-{traj}/geon-info.txt" for inv_type_name in inv_type_names ]
    for inv_type in [ 0, 1, ]:
        if get_load_path(path_f_list[inv_type]) is None:
            inv_type_name = inv_type_names[inv_type]
            q.displayln_info(f"WARNING: {fname}: {job_tag} {traj} {inv_type_name} full prop is not available yet.")
            return None
    if q.obtain_lock(f"locks/{job_tag}-{traj}-sel-from-wsrc-prop-full"):
        assert get_load_path(fn_fsel_weight) is None
        assert get_load_path(fn_fsel_rand) is None
        assert get_load_path(fn_fsel) is None
        assert get_load_path(fn_psel) is None
        assert get_load_path(fn_fsel_prob) is None
        assert get_load_path(fn_psel_prob) is None
        wi = get_wi()
        weight_min = get_param(job_tag, "field-selection-weight-minimum", default=0.3)
        fsel_rate = get_param(job_tag, "field-selection-fsel-rate")
        psel_rate = get_param(job_tag, "field-selection-psel-rate")
        q.displayln_info(-1, fname, f"fsel_rate = {fsel_rate}")
        q.displayln_info(-1, fname, f"psel_rate = {psel_rate}")
        assert fsel_rate is not None
        assert psel_rate is not None
        prop_nf_dict = dict()
        f_rand_01 = q.FieldRealD(geo, 1)
        rs = q.RngState(f"{job_tag}-{traj}").split("run_sel_from_wsrc_prop_full").split("f_rand_01")
        f_rand_01.set_rand(rs, 1.0, 0.0)
        f_rand_01.save_double(get_save_path(fn_fsel_rand))
        for inv_type in [ 0, 1, ]:
            path_f = path_f_list[inv_type]
            sfr = q.open_fields(get_load_path(path_f), "r")
            available_tags = sfr.list()
            q.displayln_info(0, f"available_tags={available_tags}")
            for idx, tslice, inv_type_wi, inv_acc in wi:
                if inv_type_wi != inv_type:
                    continue
                if inv_acc != 1:
                    continue
                tag = f"tslice={tslice} ; type={inv_type} ; accuracy={inv_acc}"
                q.displayln_info(0, f"{fname}: idx={idx} tag='{tag}'")
                if not sfr.has(tag):
                    raise Exception(f"{tag} not in {sfr.list()}")
                prop_nf = q.FieldRealD(geo, 1)
                q.set_zero(prop_nf)
                prop_nf.load_double(sfr, tag + " ; qnorm_field")
                prop_nf_dict[(inv_type, tslice,)] = prop_nf, prop_nf.glb_sum_tslice()
        f_weight_avg, f_weight_final = avg_weight_from_prop_full(geo, prop_nf_dict, weight_min)
        for inv_type in [ 0, 1, ]:
            inv_type_name = inv_type_names[inv_type]
            f_weight_avg[inv_type].save_double(get_save_path(f"{job_tag}/field-selection-weight/traj-{traj}/weight-{inv_type_name}.field"))
            q.displayln_info(-1, fname, "field-selection-weight", inv_type_name, f_weight_avg[inv_type].glb_sum_tslice()[:].ravel())
        f_weight_final.save_double(get_save_path(fn_fsel_weight))
        q.displayln_info(-1, fname, "field-selection-weight final", f_weight_final.glb_sum_tslice()[:].ravel())
        fsel = make_fsel_from_weight(f_weight_final, f_rand_01, fsel_rate)
        psel = make_psel_from_weight(f_weight_final, f_rand_01, psel_rate)
        fsel.save(get_save_path(fn_fsel))
        psel.save(get_save_path(fn_psel))
        fsel_prob = q.SelectedFieldRealD(fsel, 1)
        fsel_prob @= f_weight_final
        fsel_prob[:] = np.minimum(1.0, fsel_prob[:] * fsel_rate)
        psel_prob = q.SelectedPointsRealD(psel, 1)
        psel_prob @= f_weight_final
        psel_prob[:] = np.minimum(1.0, psel_prob[:] * psel_rate)
        fsel_prob.save_double(get_save_path(fn_fsel_prob))
        psel_prob.save(get_save_path(fn_psel_prob))
        q.release_lock()
        return ret

# -----------------------------------------------------------------------------

@q.timer_verbose
def run_prop_wsrc_sparse(job_tag, traj, *, inv_type, get_gt, get_psel, get_fsel, get_wi):
    fname = q.get_fname()
    if None in [ get_gt, get_psel, get_fsel, ]:
        return
    inv_type_names = [ "light", "strange", ]
    inv_type_name = inv_type_names[inv_type]
    path_f = f"{job_tag}/prop-wsrc-full-{inv_type_name}/traj-{traj}/geon-info.txt"
    path_s = f"{job_tag}/prop-wsrc-{inv_type_name}/traj-{traj}"
    path_sp = f"{job_tag}/psel-prop-wsrc-{inv_type_name}/traj-{traj}"
    if get_load_path(path_f) is None:
        q.displayln_info(f"WARNING: {fname}: {job_tag} {traj} {inv_type_name} full prop is not available yet.")
        return
    if get_load_path(path_s + "/geon-info.txt") is not None:
        return
    if q.obtain_lock(f"locks/{job_tag}-{traj}-wsrc-{inv_type_name}"):
        total_site = q.Coordinate(get_param(job_tag, "total_site"))
        geo = q.Geometry(total_site, 1)
        gt = get_gt()
        fsel = get_fsel()
        psel = get_psel()
        wi = get_wi()
        finished_tags = q.properly_truncate_fields(get_save_path(path_s + ".acc"))
        q.displayln_info(0, f"finished_tags={finished_tags}")
        sfr = q.open_fields(get_load_path(path_f), "r")
        available_tags = sfr.list()
        q.displayln_info(0, f"available_tags={available_tags}")
        sfw = q.open_fields(get_save_path(path_s + ".acc"), "a", q.Coordinate([ 2, 2, 2, 4, ]))
        prop = q.Prop(geo)
        s_prop = q.SelProp(fsel)
        ps_prop = q.PselProp(psel)
        for idx, tslice, inv_type_wi, inv_acc in wi:
            if inv_type_wi != inv_type:
                continue
            tag = f"tslice={tslice} ; type={inv_type} ; accuracy={inv_acc}"
            if tag in finished_tags:
                continue
            q.displayln_info(-1, f"{fname}: idx={idx} tag='{tag}'")
            if not sfr.has(tag):
                raise Exception(f"{tag} not in {sfr.list()}")
            q.set_zero(prop)
            q.set_zero(s_prop)
            q.set_zero(ps_prop)
            prop.load_double(sfr, tag)
            s_prop @= prop
            ps_prop @= prop
            ps_prop_ws = prop.glb_sum_tslice()
            fn_sp = os.path.join(path_sp, f"{tag}.lat")
            fn_spw = os.path.join(path_sp, f"{tag} ; wsnk.lat")
            ps_prop.save(get_save_path(fn_sp))
            ps_prop_ws.save(get_save_path(fn_spw))
            s_prop.save_float_from_double(sfw, tag)
            sfw.flush()
        q.qrename_info(get_save_path(path_s + ".acc"), get_save_path(path_s))
        q.release_lock()

# -----------------------------------------------------------------------------

@q.timer_verbose
def compute_prop_1(inv, src, *, tag, sfw, path_sp, psel, fsel):
    fn_sp = os.path.join(path_sp, f"{tag}.lat")
    fn_spw = os.path.join(path_sp, f"{tag} ; wsnk.lat")
    sol = inv * src
    sp_sol = q.PselProp(psel)
    sp_sol @= sol
    sp_sol.save(get_save_path(fn_sp))
    sol_ws = sol.glb_sum_tslice()
    sol_ws.save(get_save_path(fn_spw))
    s_sol = q.SelProp(fsel)
    s_sol @= sol
    s_sol.save_float_from_double(sfw, tag)
    sfw.flush()
    return sol

@q.timer
def compute_prop_wsrc(gf, gt, tslice, job_tag, inv_type, inv_acc, *,
        idx, sfw, path_sp, psel, fsel, eig, finished_tags):
    tag = f"tslice={tslice} ; type={inv_type} ; accuracy={inv_acc}"
    if tag in finished_tags:
        return None
    q.check_stop()
    q.check_time_limit()
    q.displayln_info(f"compute_prop_wsrc: idx={idx} tslice={tslice}", job_tag, inv_type, inv_acc)
    inv = ru.get_inv(gf, job_tag, inv_type, inv_acc, gt=gt, eig=eig)
    total_site = q.Coordinate(get_param(job_tag, "total_site"))
    geo = q.Geometry(total_site, 1)
    src = q.mk_wall_src(geo, tslice)
    prop = compute_prop_1(inv, src, tag=tag, sfw=sfw, path_sp=path_sp,
                          psel=psel, fsel=fsel)

@q.timer_verbose
def compute_prop_wsrc_all(job_tag, traj, *,
                          inv_type, gf, gt, wi, psel, fsel, eig):
    inv_type_names = [ "light", "strange", ]
    inv_type_name = inv_type_names[inv_type]
    path_s = f"{job_tag}/prop-wsrc-{inv_type_name}/traj-{traj}"
    path_sp = f"{job_tag}/psel-prop-wsrc-{inv_type_name}/traj-{traj}"
    finished_tags = q.properly_truncate_fields(get_save_path(path_s + ".acc"))
    sfw = q.open_fields(get_save_path(path_s + ".acc"), "a", q.Coordinate([ 2, 2, 2, 4, ]))
    for inv_acc in [ 2, 1, ]:
        for p in wi:
            idx, tslice, inv_type_p, inv_acc_p=p
            if inv_type_p == inv_type and inv_acc_p == inv_acc:
                compute_prop_wsrc(gf, gt, tslice, job_tag, inv_type, inv_acc,
                        idx=idx, sfw=sfw, path_sp=path_sp,
                        psel=psel, fsel=fsel, eig=eig,
                        finished_tags=finished_tags)
    sfw.close()
    q.qtouch_info(get_save_path(os.path.join(path_sp, "checkpoint.txt")))
    # q.qtouch_info(get_save_path(os.path.join(path_sp, "checkpoint ; wsnk.txt")))
    q.qrename_info(get_save_path(path_s + ".acc"), get_save_path(path_s))
    q.qar_create_info(get_save_path(path_sp + ".qar"), get_save_path(path_sp), is_remove_folder_after=True)
    # q.qar_create_info(get_save_path(path_s + ".qar"), get_save_path(path_s), is_remove_folder_after=True)

@q.timer
def run_prop_wsrc(job_tag, traj, *, inv_type, get_gf, get_eig, get_gt, get_psel, get_fsel, get_wi):
    """
    Can use `run_prop_wsrc_sparse` instead.
    #
    run_prop_wsrc_full(job_tag, traj, inv_type=0, get_gf=get_gf, get_eig=get_eig, get_gt=get_gt, get_wi=get_wi)
    get_fsel, get_psel, get_fsel_prob, get_psel_prob = run_fsel_psel_from_wsrc_prop_full(job_tag, traj, get_wi=get_wi)
    run_prop_wsrc_sparse(job_tag, traj, inv_type=0, get_gt=get_gt, get_psel=get_psel, get_fsel=get_fsel, get_wi=get_wi)
    """
    if None in [ get_gf, get_gt, get_psel, get_fsel, ]:
        return
    if get_eig is None:
        if inv_type == 0:
            return
        get_eig = lambda: None
    inv_type_names = [ "light", "strange", ]
    inv_type_name = inv_type_names[inv_type]
    if get_load_path(f"{job_tag}/prop-wsrc-{inv_type_name}/traj-{traj}/geon-info.txt") is not None:
        return
    if q.obtain_lock(f"locks/{job_tag}-{traj}-wsrc-{inv_type_name}"):
        gf = get_gf()
        gt = get_gt()
        eig = get_eig()
        psel = get_psel()
        fsel = get_fsel()
        assert fsel.is_containing(psel)
        wi = get_wi()
        compute_prop_wsrc_all(job_tag, traj,
                              inv_type=inv_type, gf=gf, gt=gt, wi=wi,
                              psel=psel, fsel=fsel, eig=eig)
        q.release_lock()

# -----------------------------------------------------------------------------

def calc_hvp_sum_tslice(chvp_16):
    """
    return ld_hvp_ts
    #
    ld_hvp_ts[t_dir, t, mu, nu]
    #
    t_dir in [ "x", "y", "z", "t", ]
    t in range(t_size)
    mu in range(4)
    nu in range(4)
    #
    `mu` for sink and `nu` for source
    t_size = max(total_site)
    #
    arr_for_t_dir.shape == (t_size, 4, 4,)
    #
    chvp_16 is a hvp field (from q.contract_chvp_16)
    """
    total_site = chvp_16.total_site()
    t_size = max(total_site)
    ld_hvp_ts = q.mk_lat_data([
        [ "t_dir", 4, [ "x", "y", "z", "t", ], ],
        [ "t", t_size, ],
        [ "mu", 4, ],
        [ "nu", 4, ],
        ])
    ld_hvp_ts.set_zero()
    ld_arr = ld_hvp_ts[:]
    assert ld_arr.shape == (4, t_size, 4, 4,)
    assert ld_arr.dtype == np.complex128
    for t_dir in range(4):
        chvp_16_ts = chvp_16.glb_sum_tslice(t_dir=t_dir)
        arr = chvp_16_ts.to_numpy()
        t_size = arr.shape[0]
        ld_arr[t_dir, :t_size] = arr.reshape(t_size, 4, 4)
    return ld_hvp_ts

@q.timer
def compute_prop_psrc_hvp_contract(
        job_tag, traj, xg_src, inv_type, inv_acc,
        *,
        prop, tag, sfw_hvp, path_hvp_ts):
    """
    # chvp_16.get_elem(x, mu * 4 + nu) is complex
    # (1) mu is the sink polarization and nu is the src polarization
    # (2) hvp field is simply the trace of the products of gamma matrix and propagators.
    #     It does not include the any minus sign (e.g. The minus sign due to the loop).
    """
    assert isinstance(xg_src, q.Coordinate)
    fn_hvp_ts = os.path.join(path_hvp_ts, f"{tag}.lat")
    chvp_16 = q.contract_chvp_16(prop, prop)
    ld_hvp_ts = calc_hvp_sum_tslice(chvp_16)
    ld_hvp_ts.save(get_save_path(fn_hvp_ts))
    if sfw_hvp is not None:
        chvp_16.save_float_from_double(sfw_hvp, tag)

# -----------------------------------------------------------------------------

@q.timer_verbose
def compute_prop_2(inv, src, *, tag, sfw, path_sp, psel, fsel,
                   f_rand_01, fsel_psrc_prop_norm_threshold, gt):
    fn_sp = os.path.join(path_sp, f"{tag}.lat")
    fn_spw = os.path.join(path_sp, f"{tag} ; wsnk.lat")
    sol = inv * src
    sp_sol = q.PselProp(psel)
    sp_sol @= sol
    sp_sol.save(get_save_path(fn_sp))
    sol_gt = gt * sol
    sol_ws = sol_gt.glb_sum_tslice()
    sol_ws.save(get_save_path(fn_spw))
    sol_ps_sel_prob = q.qnorm_field(sol)
    sol_ps_sel_prob *= 1.0 / fsel_psrc_prop_norm_threshold
    sol_ps_sel_prob[:] = np.minimum(1.0, sol_ps_sel_prob[:])
    ps_sel = f_rand_01[:, 0] <= sol_ps_sel_prob[:, 0]
    fsel_ps = q.FieldSelection(fsel.geo())
    fsel_ps[ps_sel] = 0
    fsel_ps.update()
    s_sol_ps_sel_prob = q.SelectedFieldRealD(fsel_ps)
    s_sol_ps_sel_prob @= sol_ps_sel_prob
    s_sol_ps_sel_prob.save_double(sfw, f"{tag} ; fsel-prob-psrc-prop")
    fsel_combine = fsel_ps.copy()
    fsel_combine.add_fsel(fsel)
    s_sol = q.SelProp(fsel_combine)
    s_sol @= sol
    s_sol.save_float_from_double(sfw, tag)
    sfw.flush()
    return sol

@q.timer
def compute_prop_psrc(job_tag, traj, xg_src, inv_type, inv_acc, *,
        idx, gf, gt, sfw, path_sp, psel, fsel, f_rand_01, sfw_hvp, path_hvp_ts, eig, finished_tags):
    assert isinstance(xg_src, q.Coordinate)
    xg = xg_src
    xg_str = f"({xg[0]},{xg[1]},{xg[2]},{xg[3]})"
    tag = f"xg={xg_str} ; type={inv_type} ; accuracy={inv_acc}"
    if tag in finished_tags:
        return None
    q.check_stop()
    q.check_time_limit()
    q.displayln_info(f"compute_prop_psrc: {job_tag} idx={idx} tag='{tag}'")
    inv = ru.get_inv(gf, job_tag, inv_type, inv_acc, eig=eig)
    total_site = q.Coordinate(get_param(job_tag, "total_site"))
    fsel_psrc_prop_norm_threshold = get_param(job_tag, "field-selection-fsel-psrc-prop-norm-threshold")
    geo = q.Geometry(total_site, 1)
    src = q.mk_point_src(geo, xg_src)
    prop = compute_prop_2(
            inv, src, tag=tag, sfw=sfw, path_sp=path_sp,
            psel=psel, fsel=fsel,
            f_rand_01=f_rand_01,
            fsel_psrc_prop_norm_threshold=fsel_psrc_prop_norm_threshold,
            gt=gt)
    compute_prop_psrc_hvp_contract(
            job_tag, traj, xg_src, inv_type, inv_acc,
            prop=prop, tag=tag, sfw_hvp=sfw_hvp, path_hvp_ts=path_hvp_ts)

@q.timer_verbose
def compute_prop_psrc_all(job_tag, traj, *,
                          inv_type, gf, gt, psel, fsel, f_rand_01, eig):
    inv_type_names = [ "light", "strange", ]
    inv_type_name = inv_type_names[inv_type]
    path_s = f"{job_tag}/prop-psrc-{inv_type_name}/traj-{traj}"
    path_s_hvp = f"{job_tag}/hvp-psrc-{inv_type_name}/traj-{traj}"
    path_hvp_ts = f"{job_tag}/hvp-sum-tslice-psrc-{inv_type_name}/traj-{traj}"
    path_sp = f"{job_tag}/psel-prop-psrc-{inv_type_name}/traj-{traj}"
    finished_tags = q.properly_truncate_fields(get_save_path(path_s + ".acc"))
    sfw = q.open_fields(get_save_path(path_s + ".acc"), "a", q.Coordinate([ 2, 2, 2, 4, ]))
    is_saving_hvp = get_param(job_tag, "run_prop_psrc", "is_saving_hvp", default=True)
    if is_saving_hvp:
        sfw_hvp = q.open_fields(get_save_path(path_s_hvp + ".acc"), "a", q.Coordinate([ 2, 2, 2, 4, ]))
    else:
        sfw_hvp = None
    def comp(idx, xg_src, inv_acc):
        compute_prop_psrc(job_tag, traj, xg_src, inv_type, inv_acc,
                idx=idx, gf=gf, gt=gt, sfw=sfw, path_sp=path_sp,
                psel=psel, fsel=fsel, f_rand_01=f_rand_01,
                sfw_hvp=sfw_hvp, path_hvp_ts=path_hvp_ts,
                eig=eig, finished_tags=finished_tags)
    prob1 = get_param(job_tag, "prob_acc_1_psrc")
    prob2 = get_param(job_tag, "prob_acc_2_psrc")
    rs = q.RngState(f"seed {job_tag} {traj}").split(f"compute_prop_psrc_all(ama)")
    for idx, xg_src in enumerate(psel):
        r = rs.split(f"{xg_src.to_tuple()}").u_rand_gen()
        assert 0 <= r and r <= 1
        comp(idx, xg_src, inv_acc=0)
        if r <= prob1:
            comp(idx, xg_src, inv_acc=1)
        if r <= prob2:
            comp(idx, xg_src, inv_acc=2)
    sfw.close()
    q.qrename_info(get_save_path(path_s + ".acc"), get_save_path(path_s))
    if sfw_hvp is not None:
        sfw_hvp.close()
        q.qrename_info(get_save_path(path_s_hvp + ".acc"), get_save_path(path_s_hvp))
    q.qtouch_info(get_save_path(os.path.join(path_sp, "checkpoint.txt")))
    q.qar_create_info(get_save_path(path_sp + ".qar"), get_save_path(path_sp), is_remove_folder_after=True)
    # q.qar_create_info(get_save_path(path_s + ".qar"), get_save_path(path_s), is_remove_folder_after=True)

@q.timer
def run_prop_psrc(job_tag, traj, *, inv_type, get_gf, get_eig, get_gt, get_psel, get_fsel, get_f_rand_01):
    if None in [ get_gf, get_gt, get_psel, get_fsel, get_f_rand_01, ]:
        return
    if get_eig is None:
        if inv_type == 0:
            return
        get_eig = lambda: None
    inv_type_names = [ "light", "strange", ]
    inv_type_name = inv_type_names[inv_type]
    if get_load_path(f"{job_tag}/prop-psrc-{inv_type_name}/traj-{traj}/geon-info.txt") is not None:
        return
    if q.obtain_lock(f"locks/{job_tag}-{traj}-psrc-{inv_type_name}"):
        gf = get_gf()
        gt = get_gt()
        eig = get_eig()
        psel = get_psel()
        fsel = get_fsel()
        f_rand_01 = get_f_rand_01()
        assert fsel.is_containing(psel)
        compute_prop_psrc_all(job_tag, traj,
                              inv_type=inv_type, gf=gf, gt=gt,
                              psel=psel, fsel=fsel,
                              f_rand_01=f_rand_01,
                              eig=eig)
        q.release_lock()

# -----------------------------------------------------------------------------

@q.timer_verbose
def compute_prop_rand_u1_type_acc(*, sfw, job_tag, traj, gf, eig, fsel, idx_rand_u1, inv_type, inv_acc, finished_tags):
    # same rand source for different inv_type
    tag = f"idx_rand_u1={idx_rand_u1} ; type={inv_type} ; accuracy={inv_acc}"
    if tag in finished_tags:
        return
    q.check_stop()
    q.check_time_limit()
    inv = ru.get_inv(gf, job_tag, inv_type, inv_acc, eig=eig)
    rs = q.RngState(f"seed {job_tag} {traj}").split(f"compute_prop_rand_u1(rand_u1)").split(str(idx_rand_u1))
    s_prop = q.mk_rand_u1_prop(inv, fsel, rs)
    s_prop.save_float_from_double(sfw, tag)
    sfw.flush()
    return s_prop

@q.timer_verbose
def compute_prop_rand_u1(*, job_tag, traj, inv_type, gf, path_s, fsel, eig=None):
    # use fsel instead of fselc
    n_rand_u1_fsel = get_param(job_tag, "n_rand_u1_fsel")
    total_site = q.Coordinate(get_param(job_tag, "total_site"))
    geo = q.Geometry(total_site, 1)
    finished_tags = q.properly_truncate_fields(get_save_path(path_s + ".acc"))
    sfw = q.open_fields(get_save_path(path_s + ".acc"), "a", q.Coordinate([ 2, 2, 2, 4, ]))
    def comp(idx_rand_u1, inv_acc):
        compute_prop_rand_u1_type_acc(
                sfw=sfw,
                job_tag=job_tag, traj=traj,
                gf=gf, eig=eig, fsel=fsel,
                idx_rand_u1=idx_rand_u1,
                inv_type=inv_type, inv_acc=inv_acc,
                finished_tags=finished_tags,)
    rs = q.RngState(f"seed {job_tag} {traj}").split(f"compute_prop_rand_u1(ama)")
    prob1 = get_param(job_tag, "prob_acc_1_rand_u1")
    prob2 = get_param(job_tag, "prob_acc_2_rand_u1")
    for idx_rand_u1 in range(n_rand_u1_fsel):
        r = rs.split(str(idx_rand_u1)).u_rand_gen()
        inv_acc = 0
        assert 0 <= r and r <= 1
        comp(idx_rand_u1, inv_acc)
        inv_acc = 1
        if r <= prob1:
            comp(idx_rand_u1, inv_acc)
        inv_acc = 2
        if r <= prob2:
            comp(idx_rand_u1, inv_acc)
    sfw.close()
    q.qrename_info(get_save_path(path_s + ".acc"), get_save_path(path_s))
    # q.qar_create_info(get_save_path(path_s + ".qar"), get_save_path(path_s), is_remove_folder_after=True)

@q.timer_verbose
def run_prop_rand_u1(job_tag, traj, *, inv_type, get_gf, get_fsel, get_eig=None):
    if None in [ get_gf, get_fsel, ]:
        return
    if get_eig is None:
        if inv_type == 0:
            return
        get_eig = lambda: None
    inv_type_names = [ "light", "strange", "charm", ]
    inv_type_name = inv_type_names[inv_type]
    path_s = f"{job_tag}/prop-rand-u1-{inv_type_name}/traj-{traj}"
    if get_load_path(path_s + "/geon-info.txt") is not None:
        return
    if q.obtain_lock(f"locks/{job_tag}-{traj}-rand-u1-{inv_type_name}"):
        gf = get_gf()
        fsel = get_fsel()
        eig = get_eig()
        compute_prop_rand_u1(
                job_tag=job_tag, traj=traj,
                inv_type=inv_type,
                gf=gf,
                path_s=path_s,
                fsel=fsel,
                eig=eig)
        q.release_lock()

# -----------------------------------------------------------------------------

@q.timer_verbose
def compute_prop_3(inv, src_smear, *, tag, sfw, path_sp, psel, fsel, gt, psel_smear, smear):
    fn_sp = os.path.join(path_sp, f"{tag}.lat")
    fn_spw = os.path.join(path_sp, f"{tag} ; wsnk.lat")
    fn_sps = os.path.join(path_sp, f"{tag} ; smear-snk.lat")
    sol = inv * src_smear
    sp_sol = q.PselProp(psel)
    sp_sol @= sol
    sp_sol.save(get_save_path(fn_sp))
    sol_gt = gt * sol
    sol_ws = sol_gt.glb_sum_tslice()
    sol_ws.save(get_save_path(fn_spw))
    sol_smear = smear(sol)
    sol_smear_psel = q.PselProp(psel_smear)
    sol_smear_psel @= sol_smear
    sol_smear_psel.save(get_save_path(fn_sps))
    s_sol = q.SelProp(fsel)
    s_sol @= sol
    s_sol.save_float_from_double(sfw, tag)
    sfw.flush()
    return sol

@q.timer
def compute_prop_smear(job_tag, xg_src, inv_type, inv_acc, *,
        idx, gf, gt, sfw, path_sp, psel, fsel, psel_smear, gf_ape, eig, finished_tags):
    xg = xg_src
    xg_str = f"({xg[0]},{xg[1]},{xg[2]},{xg[3]})"
    tag = f"smear ; xg={xg_str} ; type={inv_type} ; accuracy={inv_acc}"
    if tag in finished_tags:
        return None
    q.check_stop()
    q.check_time_limit()
    q.displayln_info(f"compute_prop_smear: {job_tag} idx={idx} tag='{tag}'")
    inv = ru.get_inv(gf, job_tag, inv_type, inv_acc, eig=eig)
    total_site = q.Coordinate(get_param(job_tag, "total_site"))
    geo = q.Geometry(total_site, 1)
    coef = get_param(job_tag, "prop_smear_coef")
    step = get_param(job_tag, "prop_smear_step")
    def smear(src):
        return q.prop_smear(src, gf_ape, coef, step)
    src = smear(q.mk_point_src(geo, xg_src))
    prop = compute_prop_3(inv, src, tag=tag, sfw=sfw, path_sp=path_sp,
                          psel=psel, fsel=fsel, gt=gt,
                          psel_smear=psel_smear, smear=smear)

@q.timer_verbose
def compute_prop_smear_all(job_tag, traj, *,
        inv_type, gf, gt, psel, fsel, psel_smear, gf_ape, eig,
        ):
    inv_type_names = [ "light", "strange", ]
    inv_type_name = inv_type_names[inv_type]
    path_s = f"{job_tag}/prop-smear-{inv_type_name}/traj-{traj}"
    path_sp = f"{job_tag}/psel-prop-smear-{inv_type_name}/traj-{traj}"
    finished_tags = q.properly_truncate_fields(get_save_path(path_s + ".acc"))
    sfw = q.open_fields(get_save_path(path_s + ".acc"), "a", q.Coordinate([ 2, 2, 2, 4, ]))
    def comp(idx, xg_src, inv_acc):
        compute_prop_smear(job_tag, xg_src, inv_type, inv_acc,
                idx=idx, gf=gf, gt=gt, sfw=sfw, path_sp=path_sp,
                psel=psel, fsel=fsel,
                psel_smear=psel_smear, gf_ape=gf_ape,
                eig=eig, finished_tags=finished_tags)
    prob1 = get_param(job_tag, "prob_acc_1_smear")
    prob2 = get_param(job_tag, "prob_acc_2_smear")
    rs = q.RngState(f"seed {job_tag} {traj}").split(f"compute_prop_smear_all(ama)")
    for idx, xg_src in enumerate(psel_smear):
        r = rs.split(f"{xg_src.to_tuple()}").u_rand_gen()
        assert 0 <= r and r <= 1
        comp(idx, xg_src, inv_acc=0)
        if r <= prob1:
            comp(idx, xg_src, inv_acc=1)
        if r <= prob2:
            comp(idx, xg_src, inv_acc=2)
    sfw.close()
    q.qtouch_info(get_save_path(os.path.join(path_sp, "checkpoint.txt")))
    q.qrename_info(get_save_path(path_s + ".acc"), get_save_path(path_s))
    q.qar_create_info(get_save_path(path_sp + ".qar"), get_save_path(path_sp), is_remove_folder_after=True)
    # q.qar_create_info(get_save_path(path_s + ".qar"), get_save_path(path_s), is_remove_folder_after=True)

@q.timer
def run_prop_smear(job_tag, traj, *, inv_type, get_gf, get_gf_ape, get_eig, get_gt, get_psel, get_fsel, get_psel_smear):
    if None in [ get_gf, get_gt, get_gf_ape, get_psel, get_fsel, ]:
        return
    if get_eig is None:
        if inv_type == 0:
            return
        get_eig = lambda: None
    inv_type_names = [ "light", "strange", ]
    inv_type_name = inv_type_names[inv_type]
    if get_load_path(f"{job_tag}/prop-smear-{inv_type_name}/traj-{traj}/geon-info.txt") is not None:
        return
    if q.obtain_lock(f"locks/{job_tag}-{traj}-smear-{inv_type_name}"):
        gf = get_gf()
        gt = get_gt()
        eig = get_eig()
        psel = get_psel()
        fsel = get_fsel()
        assert fsel.is_containing(psel)
        psel_smear = get_psel_smear()
        gf_ape = get_gf_ape()
        compute_prop_smear_all(job_tag, traj,
                inv_type=inv_type, gf=gf, gf_ape=gf_ape, gt=gt,
                psel=psel, fsel=fsel, eig=eig, psel_smear=psel_smear)
        q.release_lock()

# -----------------------------------------------------------------------------
